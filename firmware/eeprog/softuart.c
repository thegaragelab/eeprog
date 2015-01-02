/*--------------------------------------------------------------------------*
* Software UART for ATtiny processors
*---------------------------------------------------------------------------*
* 14-Apr-2014 ShaneG
*
* Provides an implementation of a UART for serial communications. Includes
* some formatted output utility functions and debug output support.
*--------------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "softuart.h"

/** Baud rate to use
 *
 * The implementation is optimised for higher baudrates - please don't use
 * anything below 57600 on an 8MHz clock. It does work at up to 250000 baud
 * but you may experience a small amount of dropped packets at that speed.
 */
#define BAUD_RATE 57600

/** Define the pin to use for transmission
 */
#define UART_TX   PINB1

/** Define the pin to use for receiving
 *
 * If this pin is the same as the TX pin the code for the single pin UART
 * implementation is compiled. This means no buffering and no interrupts.
 */
#define UART_RX   PINB0

// Calculate delays for the bit bashing functions
#ifdef F_CPU
/* account for integer truncation by adding 3/2 = 1.5 */
#  define TXDELAY   (int)(((F_CPU/BAUD_RATE)-7 +1.5)/3)
#  define RXDELAY   (int)(((F_CPU/BAUD_RATE)-5 +1.5)/3)
#  define RXDELAY2  (int)((RXDELAY*1.5)-2.5)
#  define RXROUNDED (((F_CPU/BAUD_RATE)-5 +2)/3)
#  if RXROUNDED > 127
#    error low baud rates unsupported - use higher BAUD_RATE
#  endif
#else
#  error CPU frequency F_CPU undefined
#endif

/** Initialise the UART
 */
void uartInit() {
  // Set as input and disable pullup
  DDRB  &= ~(1 << UART_RX);
  PORTB &= ~(1 << UART_RX);
  // Set up TX pin
  DDRB |= (1 << UART_TX);
  PORTB |= (1 << UART_TX);
  }

/** Write a single character
 *
 * Send a single character on the UART.
 *
 * @param ch the character to send.
 */
void __attribute__ ((noinline)) uartWrite(uint8_t ch) {
  // Set to output state and bring high
  PORTB |= (1 << UART_TX);
  cli();
  asm volatile(
    "  cbi %[uart_port], %[uart_pin]    \n\t"  // start bit
    "  in r0, %[uart_input]             \n\t"
    "  ldi r30, 3                       \n\t"  // stop bit + idle state
    "  ldi r28, %[txdelay]              \n\t"
    "TxLoop:                            \n\t"
    // 8 cycle loop + delay - total = 7 + 3*r22
    "  mov r29, r28                     \n\t"
    "TxDelay:                           \n\t"
    // delay (3 cycle * delayCount) - 1
    "  dec r29                          \n\t"
    "  brne TxDelay                     \n\t"
    "  bst %[ch], 0                     \n\t"
    "  bld r0, %[uart_pin]              \n\t"
    "  lsr r30                          \n\t"
    "  ror %[ch]                        \n\t"
    "  out %[uart_port], r0             \n\t"
    "  brne TxLoop                      \n\t"
    :
    : [uart_port] "I" (_SFR_IO_ADDR(PORTB)),
      [uart_input] "I" (_SFR_IO_ADDR(PINB)),
      [uart_pin] "I" (UART_TX),
      [txdelay] "I" (TXDELAY),
      [ch] "r" (ch)
    : "r0","r28","r29","r30");
  sei();
  }

/** Receive a single character
 *
 * Wait for a single character on the UART and return it.
 *
 * @return the character received.
 */
uint8_t __attribute__ ((noinline)) uartRead() {
  uint8_t ch;
  // Set as input and disable pullup
  DDRB  &= ~(1 << UART_RX);
  PORTB &= ~(1 << UART_RX);
  // Read the byte
  cli();
  asm volatile(
    "  ldi r18, %[rxdelay2]              \n\t" // 1.5 bit delay
    "  ldi %0, 0x80                      \n\t" // bit shift counter
    "WaitStart:                          \n\t"
    "  sbic %[uart_port]-2, %[uart_pin]  \n\t" // wait for start edge
    "  rjmp WaitStart                    \n\t"
    "RxBit:                              \n\t"
    // 6 cycle loop + delay - total = 5 + 3*r22
    // delay (3 cycle * r18) -1 and clear carry with subi
    "  subi r18, 1                       \n\t"
    "  brne RxBit                        \n\t"
    "  ldi r18, %[rxdelay]               \n\t"
    "  sbic %[uart_port]-2, %[uart_pin]  \n\t" // check UART PIN
    "  sec                               \n\t"
    "  ror %0                            \n\t"
    "  brcc RxBit                        \n\t"
    "StopBit:                            \n\t"
    "  dec r18                           \n\t"
    "  brne StopBit                      \n\t"
    : "=r" (ch)
    : [uart_port] "I" (_SFR_IO_ADDR(PORTB)),
      [uart_pin] "I" (UART_RX),
      [rxdelay] "I" (RXDELAY),
      [rxdelay2] "I" (RXDELAY2)
    : "r0","r18","r19");
  sei();
  return ch;
  }

/** Print a string from RAM
 *
 * This function simply prints the nul terminated string from RAM.
 *
 * @param cszString pointer to a character array in RAM.
 */
void uartPrint(const char *cszString) {
  for(;*cszString; cszString++)
    uartWrite(*cszString);
  }

/** Print a string from PROGMEM
 *
 * This function simply prints the nul terminated string from PROGMEM.
 *
 * @param cszString pointer to a character array in PROGMEM.
 */
void uartPrintP(const char *cszString) {
  uint8_t ch;
  while((ch = pgm_read_byte_near((uint16_t)cszString))!=0) {
    uartWrite(ch);
    cszString++;
    }
  }

/** Get the hex digit for the given value
 *
 * @param value the value to get the hext digit for
 *
 * @return the ASCII code for the hex digit.
 */
static uint8_t getHexDigit(uint8_t value) {
  value = value & 0x0F;
  if(value<10)
    return '0' + value;
  return 'a' + value - 10;
  }

/** Print an unsigned 16 bit value in hexadecimal
 *
 * Print the given value in hexadecimal format.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param value the value to print.
 * @param digits the number of digits to use for display.
 */
void uartPrintHex(uint16_t value, uint8_t digits) {
  // TODO: Should check for digit count max and min
  while(digits) {
    uartWrite(getHexDigit(value >> ((digits - 1) * 4)));
    digits--;
    }
  }

/** Print an unsigned 16 bit value in decimal
 *
 * Print the given value in decimal format.
 *
 * @param value the value to print.
 */
void uartPrintInt(uint16_t value) {
  bool emit = false;
  // Special case for 0
  if(value==0) {
    uartWrite('0');
    return;
    }
  // Emit the value, skip leading zeros
  uint16_t divisor;
  for(divisor = 10000; divisor > 0; divisor = divisor / 10) {
    uint8_t digit = value / divisor;
    value = value % divisor;
    if((digit>0)||emit) {
      uartWrite('0' + digit);
      emit = true;
      }
    }
  }

/** Helper function for formatted string output
 *
 * This function uses the current two characters of the input string to
 * determine what to send to the serial port.
 *
 * @param ch1 the current character of the format string
 * @param ch2 the next character of the format string
 * @param args the argument list containing the embedded items
 *
 * @return true if both characters should be skipped, false if we only need
 *              to move ahead by one.
 */
static bool printFormat(char ch1, char ch2, va_list *args) {
  bool skip = true;
  // Fail fast
  if(ch1=='%') {
    // Use the second character to determine what is requested
    if((ch2=='%')||(ch2=='\0'))
      uartWrite('%');
    else if(ch2=='c')
      uartWrite(va_arg(*args, int));
    else if(ch2=='u')
      uartPrintInt(va_arg(*args, unsigned int));
    else if(ch2=='x')
      uartPrintHex(va_arg(*args, unsigned int), 2);
    else if(ch2=='X')
      uartPrintHex(va_arg(*args, unsigned int), 4);
    else if(ch2=='s')
      uartPrint(va_arg(*args, char *));
    else if(ch2=='S')
      uartPrintP(va_arg(*args, char *));
    }
  else {
    uartWrite(ch1);
    skip = false;
    }
  return skip;
  }

/** Print a formatted string from RAM
 *
 * Prints a formatted string from RAM. This function supports a subset of the
 * 'printf' string formatting syntax. Allowable insertion types are:
 *
 *  %% - Display a % character. There should be no entry in the variable
 *       argument list for this entry.
 *  %u - Display an unsigned integer in decimal. The matching argument may
 *       be any 16 bit value.
 *  %x - Display an unsigned byte in hexadecimal. The matching argument may
 *       be any 8 bit value.
 *  %X - Display an unsigned word in hexadecimal. The matching argument may
 *       be any 16 bit value.
 *  %c - Display a single ASCII character. The matching argument may be any 8
 *       bit value.
 *  %s - Display a NUL terminated string from RAM. The matching argument must
 *       be a pointer to a RAM location.
 *  %S - Display a NUL terminated string from PROGMEM. The matching argument
 *       must be a pointer to a PROGMEM location.
 *
 * @param cszString pointer to a nul terminated format string in RAM.
 */
void uartFormat(const char *cszString, ...) {
  va_list args;
  va_start(args, cszString);
  char ch1, ch2 = *cszString;
  int index;
  for(index=1; ch2!='\0'; index++) {
    ch1 = ch2;
    ch2 = cszString[index];
    if(printFormat(ch1, ch2, &args)) {
      // Move ahead an extra character so we wind up jumping by two
      ch1 = ch2;
      ch2 = cszString[++index];
      }
    }
  va_end(args);
  }

/** Print a formatted string from flash (PROGMEM on AVR)
 *
 * Prints a formatted string from flash. This function supports a subset of the
 * 'printf' string formatting syntax. Allowable insertion types are:
 *
 *  %% - Display a % character. There should be no entry in the variable
 *       argument list for this entry.
 *  %u - Display an unsigned integer in decimal. The matching argument may
 *       be any 16 bit value.
 *  %x - Display an unsigned byte in hexadecimal. The matching argument may
 *       be any 8 bit value.
 *  %X - Display an unsigned word in hexadecimal. The matching argument may
 *       be any 16 bit value.
 *  %c - Display a single ASCII character. The matching argument may be any 8
 *       bit value.
 *  %s - Display a NUL terminated string from RAM. The matching argument must
 *       be a pointer to a RAM location.
 *  %S - Display a NUL terminated string from PROGMEM. The matching argument
 *       must be a pointer to a PROGMEM location.
 *
 * @param cszString pointer to a nul terminated format string in PROGMEM.
 */
void uartFormatP(const char *cszString, ...) {
  va_list args;
  va_start(args, cszString);
  char ch2 = pgm_read_byte_near((uint16_t)cszString), ch1 = ch2;
  int index;
  for(index=1; ch2!='\0'; index++) {
    ch1 = ch2;
    ch2 = pgm_read_byte_near((uint16_t)(cszString + index));
    if(printFormat(ch1, ch2, &args)) {
      // Move ahead an extra character so we wind up jumping by two
      index++;
      ch1 = ch2;
      ch2 = pgm_read_byte_near((uint16_t)(cszString + index));
      }
    }
  va_end(args);
  }

