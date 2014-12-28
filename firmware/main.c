/*--------------------------------------------------------------------------*
* Main program
*---------------------------------------------------------------------------*
* 14-Apr-2014 ShaneG
*
* Template program for ATtiny85 C/asm projects.
*--------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <microboard.h>
#include <softuart.h>
#include <softspi.h>
#include <mbstring.h>

// Program banner
#define BANNER PSTR("EEPROG V0.1\n")

// Pin definitions
enum _PINDEF {
  // SPI pins
  MISO = 10,
  MOSI = 13,
  SCK  = 12,
  CS   = 11,
  // Power control
  PWR_SPI = 6,
  PWR_I2C = 5,
  };

//! End of line character
#define EOL '\n'

//! Maximum data bytes per line
#define BYTES_PER_LINE 32

/** Maximum line length in characters
 *
 * A line is made up of a command, a 3 byte address, the data block, a
 * checksum and finally an LF character.
 */
#define LINE_LENGTH (1 + 6 + (2 * BYTES_PER_LINE) + 4 + 1)

//! Maximum supported page size
#define MAX_PAGE_SIZE 256

//! The buffer to hold the data for the current page
#define BUFFER_SIZE (MAX_PAGE_SIZE + BYTES_PER_LINE)

/** Supported commands
 *
 * Each line received by the programmer starts with a single letter command,
 * these are what we support.
 */
typedef enum {
  CMD_RESET = '!', //!< Reset the device, clear all settings
  CMD_INIT  = 'i', //!< Initialise and set the target device.
  CMD_READ  = 'r', //!< Read data from EEPROM
  CMD_WRITE = 'w', //!< Write data to EEPROM
  CMD_DONE  = 'd', //!< Done. Flush any pending data
  } COMMAND;

/** Possible modes
 */
typedef enum {
  MODE_WAITING, //!< Waiting for initialisation
  MODE_READY,   //!< Read to start writing
  MODE_WRITING, //!< Writing to EEPROM
  } MODE;

/** Masks and shifts to interpret the device configuration word
 */
typedef enum {
  // EEPROM type information
  EEPROM_TYPE_SPI = 0,
  EEPROM_TYPE_I2C = 1,
  EEPROM_TYPE_MASK = 0x8000,
  EEPROM_TYPE_SHIFT = 15,
  // Page size information
  EEPROM_PAGE_BITS_MASK = 0x7000,
  EEPROM_PAGE_BITS_SHIFT = 12,
  // EEPROM size information
  EEPROM_SIZE_BITS_MASK = 0x0F80,
  EEPROM_SIZE_BITS_SHIFT = 7,
  // EEPROM address byte count
  EEPROM_ADDR_BYTES_MASK = 0x0070,
  EEPROM_ADDR_BYTES_SHIFT = 4,
  // Extra bits (reserved)
  EEPROM_RESERVED_MASK = 0x000f,
  EEPROM_RESERVED_SHIFT= 0,
  } CHIP_IDENT;

//! The line input buffer
static uint8_t s_szLine[LINE_LENGTH];

//--- Write buffer management
static uint8_t  s_buffer[BUFFER_SIZE]; //!< The buffer itself
static uint32_t s_buffBase;            //!< Base address of buffer
static uint16_t s_buffIndex;           //!< Current index into buffer

//--- Chip characteristics
static bool     s_spi;          //!< True for SPI interface
static uint16_t s_pageSize;     //!< Size of a page in bytes
static uint8_t  s_addrBytes;    //!< Number of bytes to send as an address
static uint32_t s_chipSize;     //!< Chip capacity in bytes

// Forward definition of 'main()'
void main() __attribute__ ((noreturn));

//---------------------------------------------------------------------------
// EEPROM Interface
//---------------------------------------------------------------------------

/** Read data from an I2C EEPROM
 *
 * @param addr the address in the EEPROM to read from
 * @param length the number of bytes to read
 * @param pBuffer pointer to the buffer to contain the data
 */
void i2cReadData(uint32_t addr, uint16_t length, uint8_t *pBuffer) {
  // TODO: Implement this
  }

/** Write a single page to an I2C EEPROM
 *
 * Writes must start at a page boundary, this function assumes the caller has
 * arranged that.
 *
 * @param addr the address in the EEPROM to write to.
 * @param pBuffer pointer to the buffer containing the data
 */
void i2cWritePage(uint32_t addr, uint8_t *pBuffer) {
  // TODO: Implement this
  }

/** Read data from an SPI EEPROM
 *
 * @param addr the address in the EEPROM to read from
 * @param length the number of bytes to read
 * @param pBuffer pointer to the buffer to contain the data
 */
void spiReadData(uint32_t addr, uint16_t length, uint8_t *pBuffer) {
  // TODO: Implement this
  }

/** Write a single page to an SPI EEPROM
 *
 * Writes must start at a page boundary, this function assumes the caller has
 * arranged that.
 *
 * @param addr the address in the EEPROM to write to.
 * @param pBuffer pointer to the buffer containing the data
 */
void spiWritePage(uint32_t addr, uint8_t *pBuffer) {
  uartFormatP(PSTR("Wrote page at 0x%X%x\n"), (uint16_t)(addr >> 16), (uint16_t)(addr & 0xffff));
  // TODO: Implement this
  }

//---------------------------------------------------------------------------
// Protocol implementation
//---------------------------------------------------------------------------

/** Calculate a 16 bit checksum of a sequence of bytes
 *
 * @param pBuffer the buffer containing the data
 * @param length the number of bytes to process
 *
 * @return the 16 bit checksum
 */
static uint16_t checksum(const uint8_t *pBuffer, uint8_t length) {
  uint16_t result = 0;
  for(uint8_t index=0; index<length; index++)
    result += (uint16_t)pBuffer[index];
  return result;
  }

/** Turn 3 bytes of data into a 32 bit address
 *
 * @param pAddr pointer to the bytes for the address (MSB first)
 *
 * @return the 32 bit address.
 */
static uint32_t getAddress(uint8_t *pAddr) {
  uint32_t result = (uint32_t)pAddr[0];
  result = (result << 8) | (uint32_t)pAddr[1];
  result = (result << 8) | (uint32_t)pAddr[2];
  return result;
  }

/** Determine if the character is a hex digit or not
 */
static bool isHex(uint8_t ch) {
  return (((ch>='0')&&(ch<='9'))||((ch>='a')&&(ch<='f'))||((ch>='A')&&(ch<='F')));
  }

/** Convert a single hex character into it's value
 *
 * @param ch the character to convert
 *
 * @return the value of the hex digit
 */
static uint8_t hexVal(char ch) {
  if((ch>='0')&&(ch<='9'))
    return ch - '0';
  if((ch>='a')&&(ch<='f'))
    return ch - 'a' + 10;
  return ch - 'A' + 10;
  }

/** Convert two hex characters into a byte value
 *
 * @param szHex pointer to the string containing the hex value
 *
 * @return the byte represented by the hex.
 */
static uint8_t hexByte(const char *szHex) {
  return (hexVal(szHex[0])<<4) + hexVal(szHex[1]);
  }

/** Read an input line
 *
 * Reads an input line into s_szLine and converts any hex data that it finds.
 *
 * @return the number of data bytes in the line (which may be zero) or 0xFF
 *         if the line is not valid.
 */
static uint8_t readLine() {
  uint8_t ch;
  uint8_t index = 0;
  // Read a line until EOL char
  while((ch=uartRead())!=EOL) {
    if((index>0)&&!isHex(ch))
      index = LINE_LENGTH;
    if(index<LINE_LENGTH)
      s_szLine[index++] = ch;
    }
  // Was it too long?
  if(index>=LINE_LENGTH)
    return 0xFF;
  // Character count must be odd
  if(!(index&0x01))
    return 0xFF;
  // Convert hex into data
  uint8_t data = 1;
  for(data = 1, ch=1; ch<index; ch += 2, data++)
    s_szLine[data] = hexByte((const char *)&s_szLine[ch]);
  return data - 1;
  }

/** Send a response to the client
 *
 * @param success if true the previous command succeeded.
 * @param cszMessage pointer to a text message (in PROGMEM) to add to the response
 */
void respond(bool success, const char *cszMessage) {
  uartFormatP(PSTR("%c%S\n"), success?'+':'-', cszMessage?cszMessage:PSTR(""));
  }

/** Perform the 'init' command
 *
 * @param data the number of data bytes provided on the line.
 *
 * @return true on success, false on failure.
 */
static bool doInit(uint8_t data) {
  if(data!=2) {
    // Want a 16 bit value
    respond(false, PSTR("16 bit device ID required."));
    return false;
    }
  uint16_t ident = ((uint16_t)s_szLine[1] << 8) | s_szLine[2];
  // Get the type of device
  uint16_t value = (ident & EEPROM_TYPE_MASK) >> EEPROM_TYPE_SHIFT;
  s_spi = (value == EEPROM_TYPE_SPI);
  // Get the size of a page (specified as bits - 1)
  value = (ident & EEPROM_PAGE_BITS_MASK) >> EEPROM_PAGE_BITS_SHIFT;
  s_pageSize = (1 << (value + 1));
  // Get the total size of the EEPROM (specified as bits - 1)
  value = (ident & EEPROM_SIZE_BITS_MASK) >> EEPROM_SIZE_BITS_SHIFT;
  s_chipSize = ((uint32_t)1 << (value + 1));
  // Get the number of bytes to use in the address
  value = (ident & EEPROM_ADDR_BYTES_MASK) >> EEPROM_ADDR_BYTES_SHIFT;
  s_addrBytes = value;
  // Make sure the reserved values are 0
  value = (ident & EEPROM_RESERVED_MASK) >> EEPROM_RESERVED_SHIFT;
  if(value) {
    respond(false, PSTR("Invalid device identifier."));
    return false;
    }
  // Respond success with some additional information.
  uartFormatP(PSTR("+%S %uKb, %u byte page.\n"), s_spi?PSTR("SPI"):PSTR("I2C"), (uint16_t)(s_chipSize >> 10), s_pageSize);
  return true;
  }

/** Perform the 'read' command
 *
 * @param data the number of data bytes provided on the line.
 *
 * @return true on success, false on failure.
 */
static bool doRead(uint8_t data) {
  // Require a 3 byte address
  if(data!=3) {
    respond(false, PSTR("Read address required."));
    return false;
    }
  // Make sure we are in range
  uint32_t addr = getAddress(&s_szLine[1]);
  if(addr>s_chipSize) {
    respond(false, PSTR("Address out of range."));
    return false;
    }
  // Read the data into the buffer
  uint16_t length = ((s_chipSize - addr)<BYTES_PER_LINE)?s_chipSize - addr:BYTES_PER_LINE;
  if(s_spi)
    spiReadData(addr, length, &s_szLine[4]);
  else
    i2cReadData(addr, length, &s_szLine[4]);
  // Calculate the checksum
  uint16_t check = checksum(&s_szLine[1], length + 3);
  s_szLine[length + 5] = (uint8_t)(check >> 8);
  s_szLine[length + 6] = (uint8_t)(check & 0xFF);
  // Send the response
  uartWrite('+');
  for(uint16_t index=1; index<(length + 7); index++)
    uartFormatP(PSTR("%x"), s_szLine[index]);
  uartWrite(EOL);
  return true;
  }

/** Perform the 'write' command
 *
 * @param data the number of data bytes provided on the line.
 * @param first true if this is the first write
 *
 * @return true on success, false on failure.
 */
static bool doWrite(uint8_t data, bool first) {
  // Make sure we have enough data
  // (must be 3 byte address, at least 1 data byte and a checksum)
  if(data<6) {
    respond(false, PSTR("Not enough data for command."));
    return false;
    }
  // Verify the checksum
  uint16_t check = checksum(&s_szLine[1], data - 2);
  if(((check >> 8)!=s_szLine[data - 1])||((check & 0xff)!=s_szLine[data])) {
    respond(false, PSTR("Invalid checksum."));
    return false;
    }
  // Get and check the address
  uint32_t addr = getAddress(&s_szLine[1]);
  uint8_t length = data - 5;
  if((addr + (uint32_t)length)>s_chipSize) {
    respond(false, PSTR("Address out of range."));
    return false;
    }
  // On first write we do some initial set up
  if(first) {
    // Initialise the buffer
    s_buffBase = addr & ~((uint32_t)s_pageSize - 1);
    s_buffIndex = (uint8_t)(addr - s_buffBase);
    // If we are starting part way through a page we prime with the
    // existing data on the chip
    if(s_buffIndex) {
      if(s_spi)
        spiReadData(s_buffBase, s_buffIndex, s_buffer);
      else
        i2cReadData(s_buffBase, s_buffIndex, s_buffer);
      }
    }
  // Data must be sequential
  if(addr!=(uint32_t)(s_buffBase + (uint32_t)s_buffIndex)) {
    respond(false, PSTR("Data is not sequential."));
    return false;
    }
  // Add the data to the buffer
  uint16_t offset;
  for(offset=0; offset<length; offset++)
    s_buffer[s_buffIndex++] = s_szLine[offset + 4];
  // Write any full pages to the chip
  while(s_buffIndex>=s_pageSize) {
    // Write the page
    if(s_spi)
      spiWritePage(s_buffBase, s_buffer);
    else
      i2cWritePage(s_buffBase, s_buffer);
    // Adust the buffer
    for(offset=s_pageSize;offset<s_buffIndex;offset++)
      s_buffer[offset - s_pageSize] = s_buffer[offset];
    s_buffBase += (uint32_t)s_pageSize;
    s_buffIndex -= s_pageSize;
    }
  respond(true, NULL);
  return true;
  }

/** Perform the 'done' command
 *
 * @param data the number of data bytes provided on the line.
 *
 * @return true on success, false on failure.
 */
static bool doDone(uint8_t data) {
  respond(false, NULL);
  return false;
  }

//---------------------------------------------------------------------------
// Hardware interface
//---------------------------------------------------------------------------

/** SPI configuration
 */
static SSPI_INTERFACE s_SPI = {
  { NULL },
  SPI_PHASE_TRAILING | SPI_POLARITY_LOW | SPI_MSB_FIRST,
  MISO,
  MOSI,
  SCK
  };

/** Initialise the hardware
 */
static void hwInit() {
  // Initialise GPIO
  pinConfig(CS, OUTPUT);
  pinWrite(CS, true);
  pinConfig(PWR_SPI, OUTPUT);
  pinWrite(PWR_SPI, false);
  pinConfig(PWR_I2C, OUTPUT);
  pinWrite(PWR_I2C, false);
  // Initialise subsystems
  uartInit();
  sspiInit(&s_SPI);
  }

//---------------------------------------------------------------------------
// Main program
//---------------------------------------------------------------------------

/** Program entry point
 */
void main()  {
  hwInit();
  uartPrintP(BANNER);
  MODE mode = MODE_WAITING;
  while(true) {
    uint8_t data = readLine();
    if(data==0xFF) // Invalid line
      respond(false, PSTR("Unrecognised command."));
    else if(s_szLine[0]==CMD_RESET) {
      // Reset module
      mode = MODE_WAITING;
      pinWrite(PWR_SPI, false);
      pinWrite(PWR_I2C, false);
      uartPrintP(BANNER);
      }
    else {
      if(mode==MODE_WAITING) {
        if(s_szLine[0]==CMD_INIT) {
          // Initialise device
          if(doInit(data)) {
            // Power on the selected device
            pinWrite(s_spi?PWR_SPI:PWR_I2C, true);
            mode = MODE_READY;
            }
          }
        else
          respond(false, PSTR("Command invalid for mode."));
        }
      else if(mode==MODE_READY) {
        if(s_szLine[0]==CMD_READ)
          doRead(data);
        else if(s_szLine[0]==CMD_WRITE) {
          if(doWrite(data, true))
            mode = MODE_WRITING;
          }
        else
          respond(false, PSTR("Command invalid for mode."));
        }
      else if(mode==MODE_WRITING) {
        if(s_szLine[0]==CMD_WRITE)
          doWrite(data, false);
        else if(s_szLine[0]==CMD_DONE) {
          if(doDone(data))
            mode = MODE_READY;
          }
        else
          respond(false, PSTR("Command invalid for mode."));
        }
      else
        respond(false, PSTR("Firmware fault, invalid mode."));
      }
    }
  }
