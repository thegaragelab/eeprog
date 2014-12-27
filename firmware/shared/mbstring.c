/*---------------------------------------------------------------------------*
* Microboard string formatting support
*----------------------------------------------------------------------------*
* 19-Nov-2014 ShaneG
*
* Implement the portable string formatting functions.
*---------------------------------------------------------------------------*/
#include <stdarg.h>
#include <microboard.h>
#include <mbstring.h>

//---------------------------------------------------------------------------
// Helper functions
//---------------------------------------------------------------------------

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

/** Do the actual formatting
 *
 * This function uses the current two characters of the input string to
 * determine what to send to the serial port.
 *
 * @param pfnWriteCh pointer to a function to write the character
 * @param ch1 the current character of the format string
 * @param ch2 the next character of the format string
 * @param args the argument list containing the embedded items
 *
 * @return true if both characters should be skipped, false if we only need
 *              to move ahead by one.
 */
static bool printFormat(FN_WRITE_CHAR pfnWriteCh, char ch1, char ch2, va_list *args) {
  bool skip = true;
  // Fail fast
  if(ch1=='%') {
    // Use the second character to determine what is requested
    if((ch2=='%')||(ch2=='\0'))
      (*pfnWriteCh)('%');
    else if(ch2=='c')
      (*pfnWriteCh)(va_arg(*args, int));
    else if(ch2=='u')
      strPrintInt(pfnWriteCh, va_arg(*args, unsigned int));
    else if(ch2=='x')
      strPrintHex(pfnWriteCh, va_arg(*args, unsigned int), 2);
    else if(ch2=='X')
      strPrintHex(pfnWriteCh, va_arg(*args, unsigned int), 4);
    else if(ch2=='s')
      strPrint(pfnWriteCh, va_arg(*args, char *));
    else if(ch2=='S')
      strPrintP(pfnWriteCh, va_arg(*args, char *));
    }
  else {
    (*pfnWriteCh)(ch1);
    skip = false;
    }
  return skip;
  }

//---------------------------------------------------------------------------
// Public API
//---------------------------------------------------------------------------

/** Print a string from RAM
 *
 * This function simply prints the nul terminated string from RAM.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param cszString pointer to a character array in RAM.
 */
void strPrint(FN_WRITE_CHAR pfnWriteCh, const char *cszString) {
  for(;*cszString; cszString++)
    (*pfnWriteCh)(*cszString);
  }

/** Print a string from PROGMEM
 *
 * This function simply prints the nul terminated string from PROGMEM.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param cszString pointer to a character array in PROGMEM.
 */
void strPrintP(FN_WRITE_CHAR pfnWriteCh, const char *cszString) {
  uint8_t ch;
  while((ch = flashReadByte((uint16_t)cszString))!=0) {
    (*pfnWriteCh)(ch);
    cszString++;
    }
  }

/** Print an unsigned 16 bit value in decimal
 *
 * Print the given value in decimal format.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param value the value to print.
 */
void strPrintInt(FN_WRITE_CHAR pfnWriteCh, uint16_t value) {
  bool emit = false;
  // Special case for 0
  if(value==0) {
    (*pfnWriteCh)('0');
    return;
    }
  // Emit the value, skip leading zeros
  for(uint16_t divisor = 10000; divisor > 0; divisor = divisor / 10) {
    uint8_t digit = value / divisor;
    value = value % divisor;
    if((digit>0)||emit) {
      (*pfnWriteCh)('0' + digit);
      emit = true;
      }
    }
  }

/** Print an unsigned 16 bit value in hexadecimal
 *
 * Print the given value in hexadecimal format.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param value the value to print.
 * @param digits the number of digits to use for display.
 */
void strPrintHex(FN_WRITE_CHAR pfnWriteCh, uint16_t value, uint8_t digits) {
  // TODO: Should check for digit count max and min
  while(digits) {
    (*pfnWriteCh)(getHexDigit(value >> ((digits - 1) * 4)));
    digits--;
    }
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
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param cszString pointer to a nul terminated format string in RAM.
 */
void strFormat(FN_WRITE_CHAR pfnWriteCh, const char *cszString, ...) {
  va_list args;
  va_start(args, cszString);
  char ch1, ch2 = *cszString;
  for(int index=1; ch2!='\0'; index++) {
    ch1 = ch2;
    ch2 = cszString[index];
    if(printFormat(pfnWriteCh, ch1, ch2, &args)) {
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
void strFormatP(FN_WRITE_CHAR pfnWriteCh, const char *cszString, ...) {
  va_list args;
  va_start(args, cszString);
  char ch2 = flashReadByte((uint16_t)cszString), ch1 = ch2;
  for(int index=1; ch2!='\0'; index++) {
    ch1 = ch2;
    ch2 = flashReadByte((uint16_t)(cszString + index));
    if(printFormat(pfnWriteCh, ch1, ch2, &args)) {
      // Move ahead an extra character so we wind up jumping by two
      index++;
      ch1 = ch2;
      ch2 = flashReadByte((uint16_t)(cszString + index));
      }
    }
  va_end(args);
  }

