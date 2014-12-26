/*---------------------------------------------------------------------------*
* Microboard basic processor support.
*----------------------------------------------------------------------------*
* 19-Nov-2014 ShaneG
*
* This is a port of my original 'tinytemplate' library for the ATtiny85 chip
* to support a larger range of target architectures and chips. It is not
* designed for 100% source compatibility between architectures - simply to
* provide a common set of functions and definitions to make moving from one
* architecture to another relatively painless.
*---------------------------------------------------------------------------*/
#ifndef __MICROBOARD_H
#define __MICROBOARD_H

// Bring in required definitions
#include <stdint.h>
#include <stdbool.h>
//#include <mbtarget.h>

// Target specific definitions
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)
#  include <targets/attiny.h>
#elif defined(__AVR_ATmega8) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)
#  include <targets/atmega.h>
#else
#  error "Unsupported target platform"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Timing functions
//---------------------------------------------------------------------------

/** Initialise the timing system
 *
 * This function must be called prior to using any of the time related
 * functions. On some platforms a hardware timer is utilised to provide the
 * functionality required and this function will do the necessary setup.
 * See the implementation notes for your target platform for details.
 *
 * @return true if the timing system was configured successfully.
 */
bool timeInit();

/** Get the system tick count.
 *
 * The timing system maintains a 'tick count' which represents the number
 * of milliseconds since power up. as a 16 bit value. This value will
 * increment every millisecond and can be used as a reference point for
 * measuring durations.
 *
 * @return the current system tick value.
 */
uint16_t time();

/** Calculate the ticks elapsed since the given sample.
 *
 * This function calculates the number of ticks that have elapsed since the
 * reference sample was taken. The function takes into account the wrap around
 * of the counter (but it cannot detect multiple wrap arounds).
 *
 * @param reference the reference tick count to calculate against.
 *
 * @return the number of ticks elapsed since the reference count.
 */
uint16_t timeElapsed(uint16_t reference);

/** Delay for a specific number of milliseconds
 *
 * This function performs a 'busy waiting' loop for the specified number of
 * milliseconds. The function has an accuracy of approximately 0.5ms.
 *
 * @param ms the number of milliseconds to delay for.
 */
void timeDelay(uint16_t ms);

//---------------------------------------------------------------------------
// Memory access
//---------------------------------------------------------------------------

/** Read a byte from flash memory.
 *
 * This is a wrapper function to allow reading data from flash memory. Some
 * architectures (like PIC and AVR) use a separate address space and therefore
 * special instructions to access that memory. This function provides a generic
 * interface to that functionality.
 *
 * @param addr the address of the data to read.
 *
 * @return the byte contained at the specified address.
 */
uint8_t flashReadByte(uint16_t addr);

/** Read a word from flash memory.
 *
 * This is a wrapper function to allow reading data from flash memory. Some
 * architectures (like PIC and AVR) use a separate address space and therefore
 * special instructions to access that memory. This function provides a generic
 * interface to that functionality.
 *
 * @param addr the address of the data to read.
 *
 * @return the word contained at the specified address.
 */
uint16_t flashReadWord(uint16_t addr);

/** Read a byte from EEPROM memory
 *
 * This is a wrapper function to read a byte from the EEPROM memory (if any)
 * on the chip. Devices with EEPROM available will define the macro
 * TARGET_HAS_EEPROM and define EEPROM_SIZE to the number of bytes of
 * EEPROM available.
 *
 * @param addr the EEPROM address to read.
 *
 * @return the byte contained at the specified address.
 */
uint8_t eepromRead(uint16_t addr);

/** Write a byte to EEPROM memory
 *
 * This is a wrapper function to write a byte to the EEPROM memory (if any)
 * on the chip. Devices with EEPROM available will define the macro
 * TARGET_HAS_EEPROM and define EEPROM_SIZE to the number of bytes of
 * EEPROM available.
 *
 * @param addr the EEPROM address to write.
 * @param val the byte to store at the address.
 */
void eepromWrite(uint16_t addr, uint8_t val);

//---------------------------------------------------------------------------
// Digital IO operations
//---------------------------------------------------------------------------

/** Pin number
 */
typedef uint8_t PIN;

/** Pin direction
 */
typedef enum __PIN_DIRECTION {
  OUTPUT = 0,      //!< Output pin
  INPUT,           //!< Input pin
  INPUT_PULLUP     //!< Input pin with internal pull up
  } PIN_DIRECTION;

/** Set up a digital pin
 *
 * Configure a pin for digital IO. The pin number is the physical pin number
 * on the CPU chip, not the pin number assigned by the architecture.
 *
 * @param pin the pin number to configure as a digital pin.
 * @param direction the direction of the pin (@see PIN_DIRECTION).
 *
 * @return true on successful configuration, false on failure.
 */
bool pinConfig(PIN pin, PIN_DIRECTION direction);

/** Change the state of an output pin
 *
 * @param pin the number of the pin to modify.
 * @param state the new state of the pin - true for on (high), false for off (low)
 */
void pinWrite(PIN pin, bool state);

/** Read the state of an input pin
 *
 * Read the input of a digital pin. This function assumes the pin has been
 * configured as a digital input - if it has not the results are undefined.
 *
 * @param pin the number of the pin to read.
 *
 * @return true if the input is high, false if the input is low.
 */
bool pinRead(PIN pin);

//---------------------------------------------------------------------------
// Analog input operations
//---------------------------------------------------------------------------

/** Set up an analog pin
 *
 * Configure the specified pin as an analog input.
 *
 * @param pin the number of the pin to configure.
 * @param options the options to apply to the ADC such as the reference
 *                voltage source. These are platform dependent.
 *
 * @return true if the pin was configured successfully.
 */
bool adcConfig(PIN pin, ADC_OPTIONS options);

/** Read an analog input
 *
 * Read the current value of the analog input with support for input correction.
 * This function allows you to skip a number of samples (to allow for settling
 * time when switching the analog input multiplexor) and to specify the number
 * of samples to average for the final value.
 *
 * @param pin the analog input to sample.
 * @param ignore the number of samples to ignore.
 * @param samples the number of samples to average.
 *
 * @return the averaged value.
 */
uint16_t adcRead(PIN pin, uint8_t ignore, uint8_t samples);

//---------------------------------------------------------------------------
// PWM output operations
//---------------------------------------------------------------------------

/** Set up a PWM pin
 *
 * Configure an output pin as a PWM output. The PWM output may be generated
 * by hardware (on platforms that support it) or by software.
 *
 * @param pin the number of the pin to configure.
 * @param options options to apply to the PWM output such as frequency. These
 *                are platform dependent.
 *
 * @return true if the pin was configured correctly.
 */
bool pwmConfig(PIN pin, PWM_OPTIONS options);

/** Write a PWM output
 *
 * Set the duty cycle for a PWM output. The function defines a 16 bit value
 * for the duty cycle where 0 is completely off and 65535 is completely on.
 * This value will be scaled to the resolution supported by the underlying
 * hardware.
 *
 * @param pin the number of the pin to modify.
 * @param value the duty cycle to apply to the PWM output.
 */
void pwmWrite(PIN pin, uint16_t value);

//---------------------------------------------------------------------------
// Hardware I2C support
//
// If the target platform has I2C hardware (TARGET_HAS_I2C is defined) these
// functions provide a simple interface to allow data transfer.
//---------------------------------------------------------------------------

// TODO: Implement this

//---------------------------------------------------------------------------
// Hardware SPI support
//
// If the target platform has SPI hardware (TARGET_HAS_SPI is defined) these
// functions provide a simple interface to allow data transfer. This section
// also defines a standard SPI device interface that can be used by drivers.
// This allows for drivers to be written that can use any implementation of
// the interface.
//---------------------------------------------------------------------------

/** SPI option flags
 *
 * Combinations of these flags are used to set the options of the SPI
 * interface.
 */
typedef enum _SPI_OPTIONS {
  // The phase option (CPHA) defines which clock edge data is latched on.
  SPI_PHASE_LEADING  = 0x00,  //!< CPHA = 0, latch on leading edge
  SPI_PHASE_TRAILING = 0x01,  //!< CPHA = 1, latch on trailing edge
  // The polarity (CPOL) defines the base value of the clock.
  SPI_POLARITY_LOW   = 0x00,  //!< CPOL = 0, clock is active high
  SPI_POLARITY_HIGH  = 0x02,  //!< CPOL = 1, clock is active low
  // The direction defines the bit order of transfers
  SPI_MSB_FIRST      = 0x00,  //!< Most significant bit sent first
  SPI_LSB_FIRST      = 0x04,  //!< Least significant bit sent first
  } SPI_OPTIONS;

/** This structure defines an interface to an SPI device.
 *
 * Alternative implementations (such as software based SPI) can be provided.
 * As long as a device driver uses the interface rather than calling SPI
 * implementation functions directly it can be used with any SPI interface.
 */
typedef struct _SPI_INTERFACE {
  /** Transfer up to 16 bits over an SPI interface.
   *
   * @param output the data to send out
   * @param bits the number of bits to transfer
   *
   * @return the data received from the SPI interface
   */
  uint16_t (*m_pfnTransfer)(struct _SPI_INTERFACE *pSPI, uint16_t output, uint8_t bits);
  } SPI_INTERFACE;

/** Initialise the SPI interface
 *
 * @return true if the interface was initialised correctly.
 */
bool spiInit(SPI_OPTIONS options);

/** Transfer up to 16 bits over an SPI interface.
 *
 * @param output the data to send out
 * @param bits the number of bits to transfer
 *
 * @return the data received from the SPI interface
 */
uint16_t spiTransfer(uint16_t output, uint8_t bits);

/** Transfer up to 16 bits over an SPI interface.
 *
 * @param output the data to send out
 * @param bits the number of bits to transfer
 *
 * @return the data received from the SPI interface
 */
uint16_t spiTransferEx(SPI_INTERFACE *pInterface, uint16_t output, uint8_t bits);

/** Initialise the SPI interface structure
 */
inline bool spiInitEx(SPI_INTERFACE *pInterface, SPI_OPTIONS options) {
  pInterface->m_pfnTransfer = spiTransferEx;
  return spiInit(options);
  }

//---------------------------------------------------------------------------
// Serial port support
//
// Support for serial communications is optional, if present (TARGET_HAS_SERIAL
// is defined) it may be implemented in software and have some limitations
// compared to a device with a hardware UART.
//---------------------------------------------------------------------------

/** Initialise the serial port
 *
 * Initialise the serial port with the specified baud rate, 8 data bits, no
 * parity and 1 stop bit. Some targets do not support hardware serial and
 * the implementation will be done in software instead.
 *
 * @param baud the baud rate to initialise the port at.
 */
bool serialInit(uint32_t baud);

/** Determine if characters are available to read on the serial port
 *
 * @return true if characters are available in the serial input buffer.
 */
bool serialAvailable();

/** Read a single character from the serial port.
 *
 * This function will block until data is available to read. Use serialAvailable()
 * to check for available characters.
 *
 * @return the character read.
 */
uint8_t serialRead();

/** Write a single character to the serial port
 *
 * @param ch the character to write on the serial port.
 */
void serialWrite(uint8_t ch);

//---------------------------------------------------------------------------
// CRC Calculations
//
// These utility functions provide a method to validate data that is stored
// or transmitted.
//---------------------------------------------------------------------------

/** Initialise the CRC calculation
 *
 * Initialises the CRC value prior to processing data.
 *
 * @return the initial CRC value.
 */
uint16_t crcInit();

/** Add a byte to an ongoing CRC calculation
 *
 * Update the CRC value with an additional data byte.
 *
 * @param crc the current CRC value
 * @param data the data byte to add to the calculation
 *
 * @return the updated CRC value.
 */
uint16_t crcByte(uint16_t crc, uint8_t data);

/** Add a block of data to an ongoing CRC calculation
 *
 * Add a sequence of bytes from a buffer in RAM. Note that this function can
 * process up to 255 bytes in a single call (which is usually enough for a
 * microcontroller application).
 *
 * @param crc the current CRC value
 * @param pData pointer to the memory buffer
 * @param length the number of bytes to process.
 *
 * @return the updated CRC value.
 */
uint16_t crcData(uint16_t crc, const uint8_t *pData, uint8_t length);

/** Add a block of data to an ongoing CRC calculation
 *
 * Add a sequence of bytes from a buffer in PROGMEM. Note that this function
 * can process up to 255 bytes in a single call (which is usually enough for a
 * microcontroller application).
 *
 * @param crc the current CRC value
 * @param pData pointer to the memory location.
 * @param length the number of bytes to process.
 *
 * @return the updated CRC value.
 */
uint16_t crcDataP(uint16_t crc, const uint8_t *pData, uint8_t length);

//---------------------------------------------------------------------------
// Configuration management
//
// These functions provide a simple way to save and load configuration data
// from the EEPROM with validation.
//---------------------------------------------------------------------------

/** Save a configuration structure to the EEPROM
 *
 * This function will write the contents of the structure pointed to by
 * pConfig to the EEPROM at addr. The function calculates a 16 bit CRC for
 * the block of data and appends it to the EEPROM as well.
 *
 * @param pConfig pointer to the configuration data.
 * @param size the size of the configuration block. The function will write
 *             this many bytes plus 2 to the EEPROM.
 * @param addr the offset into the EEPROM to start writing.
 */
void configSave(void *pConfig, uint16_t size, uint16_t addr);

/** Load a configuration structure from the EEPROM
 *
 * This function will read from the EEPROM into the memory location pointed
 * to by pConfig. The function calculates a 16 bit CRC for the block of data
 * and verifies it against the CRC stored in the EEPROM.
 *
 * @param pConfig pointer to the configuration data.
 * @param size the size of the configuration block. The function will read
 *             this many bytes from the EEPROM.
 * @param addr the offset into the EEPROM to start reading.
 *
 * @return true if the data was read and the CRC matched.
 */
bool configLoad(void *pConfig, uint16_t size, uint16_t addr);

#ifdef __cplusplus
}
#endif

#endif /* __MICROBOARD_H */

