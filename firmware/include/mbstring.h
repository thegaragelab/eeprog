/*---------------------------------------------------------------------------*
* Microboard string formatting support
*----------------------------------------------------------------------------*
* 19-Nov-2014 ShaneG
*
* Provide simple string formatting functions. This also defines formatted
* output functions for the serial port.
*---------------------------------------------------------------------------*/
#ifndef __MBSTRING_H
#define __MBSTRING_H

//--- Bring in required definitions
#include <microboard.h>

#ifdef __cplusplus
extern "C" {
#endif

/** An interface function to write a single character
 */
typedef void (*FN_WRITE_CHAR)(uint8_t ch);

//---------------------------------------------------------------------------
// Basic output for data types.
//---------------------------------------------------------------------------

/** Print a string from RAM
 *
 * This function simply prints the nul terminated string from RAM.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param cszString pointer to a character array in RAM.
 */
void strPrint(FN_WRITE_CHAR pfnWriteCh, const char *cszString);

//! Helper macro to print to the serial port
#define serialPrint(cszString) strPrint(serialWrite, cszString)

/** Print a string from PROGMEM
 *
 * This function simply prints the nul terminated string from PROGMEM.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param cszString pointer to a character array in PROGMEM.
 */
void strPrintP(FN_WRITE_CHAR pfnWriteCh, const char *cszString);

//! Helper macro to print to the serial port
#define serialPrintP(cszString) strPrintP(serialWrite, cszString)

/** Print an unsigned 16 bit value in decimal
 *
 * Print the given value in decimal format.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param value the value to print.
 */
void strPrintInt(FN_WRITE_CHAR pfnWriteCh, uint16_t value);

//! Helper macro to print to the serial port
#define serialPrintInt(value) strPrintInt(serialWrite, value)

/** Print an unsigned 16 bit value in hexadecimal
 *
 * Print the given value in hexadecimal format.
 *
 * @param pfnWriteCh pointer to a function to write individual characters
 * @param value the value to print.
 * @param digits the number of digits to use for display.
 */
void strPrintHex(FN_WRITE_CHAR pfnWriteCh, uint16_t value, uint8_t digits);

//! Helper macro to print to the serial port
#define serialPrintHex(value) strPrintHex(serialWrite, value)

//---------------------------------------------------------------------------
// Formatted output
//---------------------------------------------------------------------------

/** Print a formatted string from RAM
 *
 * Prints a formatted string from RAM. This function supports a subset of the
 * 'printf' string formatting syntax. Allowable insertion types are:
 *
 *  %% - Display a % character. There should be no entry in the variable
 *       argument list for this entry.
 *  %u - Display an unsigned integer in decimal. The matching argument may
 *       be any 16 bit value.
 *  %x - Display an unsigned integer in hexadecimal. The matching argument may
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
void strFormat(FN_WRITE_CHAR pfnWriteCh, const char *cszString, ...);

//! Helper macro to print to the serial port
#define serialFormat(format, ...) strFormat(serialWrite, format, __VA_ARGS__)

/** Print a formatted string from flash (PROGMEM on AVR)
 *
 * Prints a formatted string from flash. This function supports a subset of the
 * 'printf' string formatting syntax. Allowable insertion types are:
 *
 *  %% - Display a % character. There should be no entry in the variable
 *       argument list for this entry.
 *  %u - Display an unsigned integer in decimal. The matching argument may
 *       be any 16 bit value.
 *  %x - Display an unsigned integer in hexadecimal. The matching argument may
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
void strFormatP(FN_WRITE_CHAR pfnWriteCh, const char *cszString, ...);

//! Helper macro to print to the serial port
#define serialFormatP(format, ...) strFormatP(serialWrite, format, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __MBSTRING_H */

