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

// Forward definition of 'main()'
void main() __attribute__ ((noreturn));

//---------------------------------------------------------------------------
// Protocol implementation
//---------------------------------------------------------------------------

//! End of line character
#define EOL '\n'

//! Maximum data bytes per line
#define BYTES_PER_LINE 32

/** Maximum line length in characters
 *
 * A line is made up of a command, a 3 byte address, the data block, a
 * checksum and finally an LF character.
 */
#define LINE_LENGTH (1 + 3 + (2 * BYTES_PER_LINE) + 2 + 1)

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

//! The data buffer
static uint8_t s_buffer[BUFFER_SIZE];

static bool     s_spi;          //!< True for SPI interface
static uint8_t  s_pageSize;     //!< Size of a page in bytes
static uint8_t  s_addrBytes;    //!< Number of bytes to send as an address
static uint32_t s_maxAddr;      //!< Highest address in chip

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
  s_pageSize = (1 << (value + 1)) - 1;
  // Get the total size of the EEPROM (specified as bits - 1)
  value = (ident & EEPROM_SIZE_BITS_MASK) >> EEPROM_SIZE_BITS_SHIFT;
  s_maxAddr = (1 << (value + 1)) - 1;
  // Get the number of bytes to use in the address
  value = (ident & EEPROM_ADDR_BYTES_MASK) >> EEPROM_ADDR_BYTES_SHIFT;
  s_addrBytes = value;
  // Make sure the reserved values are 0
  value = (ident & EEPROM_RESERVED_MASK) >> EEPROM_RESERVED_SHIFT;
  if(value) {
    respond(false, PSTR("Invalid device identifier."));
    return false;
    }
  // TODO: Would be nice to include information in the response line
  respond(true, NULL);
  return true;
  }

/** Perform the 'read' command
 *
 * @param data the number of data bytes provided on the line.
 *
 * @return true on success, false on failure.
 */
static bool doRead(uint8_t data) {
  respond(false, NULL);
  return false;
  }

/** Perform the 'write' command
 *
 * @param data the number of data bytes provided on the line.
 *
 * @return true on success, false on failure.
 */
static bool doWrite(uint8_t data) {
  respond(false, NULL);
  return false;
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
      respond(false, PSTR("Invalid command"));
    else if(s_szLine[0]==CMD_RESET) {
      // Reset module
      mode = MODE_WAITING;
      uartPrintP(BANNER);
      }
    else {
      if(mode==MODE_WAITING) {
        if(s_szLine[0]==CMD_INIT) {
          // Initialise device
          if(doInit(data))
            mode = MODE_READY;
          }
        else
          respond(false, PSTR("Command invalid for mode."));
        }
      else if(mode==MODE_READY) {
        if(s_szLine[0]==CMD_READ)
          doRead(data);
        else if(s_szLine[0]==CMD_WRITE) {
          if(doWrite(data))
            mode = MODE_WRITING;
          }
        else
          respond(false, PSTR("Command invalid for mode."));
        }
      else if(mode==MODE_WRITING) {
        if(s_szLine[0]==CMD_WRITE)
          doWrite(data);
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
