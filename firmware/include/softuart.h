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

/** Determine if characters are available
 *
 * Check the number of characters available for immediate reading. If this
 * function returns a non-zero value the next call to uartRecv() will be
 * guaranteed to return immediately with a value. This function is only valid
 * if you are using the interrupt driven version of the UART.
 *
 * @return the number of characters available in the input buffer.
 */
uint8_t uartAvail();

/** Receive a single character
 *
 * Wait for a single character on the UART and return it. If data is not
 * immediately available this function will block until data has been
 * received.
 *
 * @return the character received.
 */
uint8_t uartRead();

//---------------------------------------------------------------------------
// Debugging output support (requires formatted print functions)
//---------------------------------------------------------------------------

#if defined(DEBUG)
#  define PRINT(msg)       uartPrintP(PSTR("DEBUG: ")), uartPrintP(PSTR(msg))
#  define PRINTF(fmt, ...) uartPrintP(PSTR("DEBUG: ")), uartFormatP(PSTR(fmt), __VA_ARGS__)
#else
#  define PRINT(msg)
#  define PRINTF(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SOFTUART_H */
