/*---------------------------------------------------------------------------*
* Portable software SPI implementation
*----------------------------------------------------------------------------*
* 20-Nov-2014 ShaneG
*
* This allows SPI communication over arbitrary digital pins. This implement
* is portable, all hardware specific operations are performed by the Microboard
* core library.
*---------------------------------------------------------------------------*/
#ifndef __SOFTSPI_H
#define __SOFTSPI_H

// Bring in required definitions
#include <microboard.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SSPI_INTERFACE {
  SPI_INTERFACE m_spi;
  SPI_OPTIONS   m_options;
  PIN           m_miso;
  PIN           m_mosi;
  PIN           m_sck;
  } SSPI_INTERFACE;

/** Initialise the SPI interface
 *
 * @return true if the interface was initialised correctly.
 */
bool sspiInit(SSPI_INTERFACE *pInterface);

/** Transfer up to 16 bits over an SPI interface.
 *
 * @param output the data to send out
 * @param bits the number of bits to transfer
 *
 * @return the data received from the SPI interface
 */
uint16_t sspiTransfer(SSPI_INTERFACE *pInterface, uint16_t output, uint8_t bits);

#ifdef __cplusplus
}
#endif

#endif /* __SOFTSPI_H */

