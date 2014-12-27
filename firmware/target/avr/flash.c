/*---------------------------------------------------------------------------*
* Flash (program memory) access for AVR
*----------------------------------------------------------------------------*
* 27-Dec-2014 ShaneG
*
* Provide read/write access to flash memory on the AVR
*---------------------------------------------------------------------------*/
#include <avr/pgmspace.h>
#include <microboard.h>

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
uint8_t flashReadByte(uint16_t addr) {
  return pgm_read_byte_near((uint8_t *)addr);
  }

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
uint16_t flashReadWord(uint16_t addr) {
  return pgm_read_word_near((uint8_t *)addr);
  }

