/*--------------------------------------------------------------------------*
* Software SPI
*---------------------------------------------------------------------------*
* 19-Apr-2014 ShaneG
*
* An implementation of SPI in software. Useful for LCD displays and other
* devices where a full hardware SPI implementation would waste IO pins.
*--------------------------------------------------------------------------*/
#include <microboard.h>
#include <softspi.h>

//---------------------------------------------------------------------------
// Helper functions
//---------------------------------------------------------------------------

/** Transfer data to and from a slave (MSB first)
 *
 * @param pInterface the pointer to the configuration interface
 * @param data the data to transfer out
 * @param bits the number of bits to transfer
 *
 * @return a 16 bit value containing the read data in the lowest bits.
 */
static uint16_t sspiTransferMSB(SSPI_INTERFACE *pInterface, uint16_t data, uint8_t bits) {
  uint16_t result = 0, mask = 1 << bits;
  bool active = !(pInterface->m_options&SPI_POLARITY_HIGH);
  if(pInterface->m_options&SPI_PHASE_TRAILING) {
    // Read data on leading edge, data output on trailing edge
    while(bits) {
      pinWrite(pInterface->m_sck, active);
      result = (result << 1) | (pinRead(pInterface->m_miso)?1:0);
      pinWrite(pInterface->m_mosi, data&mask);
      // TODO: Is a delay needed here?
      pinWrite(pInterface->m_sck, !active);
      bits--;
      mask = mask >> 1;
      }
    }
  else {
    // Read data on trailing edge, data output on leading edge
    while(bits) {
      pinWrite(pInterface->m_mosi, data&mask);
      pinWrite(pInterface->m_sck, active);
      // TODO: Is a delay needed here?
      pinWrite(pInterface->m_sck, !active);
      result = (result << 1) | (pinRead(pInterface->m_miso)?1:0);
      bits--;
      mask = mask >> 1;
      }
    }
  return result;
  }

/** Transfer data to and from a slave (LSB first)
 *
 * @param pInterface the pointer to the configuration interface
 * @param data the data to transfer out
 * @param bits the number of bits to transfer
 *
 * @return a 16 bit value containing the read data in the lowest bits.
 */
static uint16_t sspiTransferLSB(SSPI_INTERFACE *pInterface, uint16_t data, uint8_t bits) {
  uint16_t result = 0;
  bool active = !(pInterface->m_options&SPI_POLARITY_HIGH);
  if(pInterface->m_options&SPI_PHASE_TRAILING) {
    // Read data on leading edge, data output on trailing edge
    while(bits) {
      pinWrite(pInterface->m_sck, active);
      result = (result >> 1) | (pinRead(pInterface->m_miso)?0x8000:0);
      pinWrite(pInterface->m_mosi, data&0x0001);
      // TODO: Is a delay needed here?
      pinWrite(pInterface->m_sck, !active);
      bits--;
      data = data >> 1;
      }
    }
  else {
    // Read data on trailing edge, data output on leading edge
    while(bits) {
      pinWrite(pInterface->m_mosi, data&0x0001);
      pinWrite(pInterface->m_sck, active);
      // TODO: Is a delay needed here?
      pinWrite(pInterface->m_sck, !active);
      result = (result >> 1) | (pinRead(pInterface->m_miso)?0x8000:0);
      bits--;
      data = data >> 1;
      }
    }
  result = result >> (16 - bits);
  return result;
  }

//---------------------------------------------------------------------------
// Public API
//---------------------------------------------------------------------------

/** Initialise the SPI interface
 *
 * @return true if the interface was initialised correctly.
 */
bool sspiInit(SSPI_INTERFACE *pInterface) {
  // Initialise the pins
  bool result = pinConfig(pInterface->m_sck, OUTPUT);
  result &= pinConfig(pInterface->m_mosi, OUTPUT);
  result &= pinConfig(pInterface->m_miso, INPUT);
  if(!result)
    return false;
  // Bring SCK to base level
  pinWrite(pInterface->m_sck, !(pInterface->m_options&SPI_POLARITY_HIGH));
  // Set up the transfer function
  pInterface->m_spi.m_pfnTransfer = sspiTransfer;
  return true;
  }

/** Transfer up to 16 bits over an SPI interface.
 *
 * @param output the data to send out
 * @param bits the number of bits to transfer
 *
 * @return the data received from the SPI interface
 */
uint16_t sspiTransfer(SSPI_INTERFACE *pInterface, uint16_t output, uint8_t bits) {
  if(pInterface->m_options&SPI_LSB_FIRST)
    return sspiTransferLSB(pInterface, output, bits);
  return sspiTransferMSB(pInterface, output, bits);
  }

