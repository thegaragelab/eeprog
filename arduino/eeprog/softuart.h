/*--------------------------------------------------------------------------*
* Software UART for ATtiny processors
*---------------------------------------------------------------------------*
* 14-Apr-2014 ShaneG
*
* Provides an implementation of a UART for serial communications. Includes
* some formatted output utility functions and debug output support.
*--------------------------------------------------------------------------*/
#ifndef __SOFTUART_H
#define __SOFTUART_H

//--- Required definitions
#include <stdint.h>
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Core operations
//---------------------------------------------------------------------------

/** Initialise the UART
 */
void uartInit();

/** Write a single character
 *
 * Send a single character on the UART.
 *
 * @param ch the character to send.
 */
void uartWrite(uint8_t ch);

/** Receive a single character
 *
 * Wait for a single character on the UART and return it. If data is not
 * immediately available this function will block until data has been
 * received.
 *
 * @return the character received.
 */
uint8_t uartRead();

/** Print a string from RAM
 *
 * This function simply prints the nul terminated string from RAM.
 *
 * @param cszString pointer to a character array in RAM.
 */
void uartPrint(const char *cszString);

/** Print a string from PROGMEM
 *
 * This function simply prints the nul terminated string from PROGMEM.
 *
 * @param cszString pointer to a character array in PROGMEM.
 */
void uartPrintP(const char *cszString);

/** Print an unsigned 16 bit value in hexadecimal
 *
 * Print the given value in hexadecimal format.
 *
 * @param value the value to print.
 * @param digits the number of digits to use for display.
 */
void uartPrintHex(uint16_t value, uint8_t digits);

/** Print an unsigned 16 bit value in decimal
 *
 * Print the given value in decimal format.
 *
 * @param value the value to print.
 */
void uartPrintInt(uint16_t value);

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
void uartFormat(const char *cszString, ...);

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
void uartFormatP(const char *cszString, ...);

#ifdef __cplusplus
}
#endif

#endif /* __SOFTUART_H */
