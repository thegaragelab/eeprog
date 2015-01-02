#include <avr/pgmspace.h>
#include "softuart.h"

// Program banner
#define BANNER PSTR("EEPROG V0.1\n")

/** Pin definitions
 * ATMEL ATTINY84 / ARDUINO
 *
 *                           +-\/-+
 *                     VCC  1|    |14  GND
 *             (D 10)  PB0  2|    |13  AREF (D  0)
 *             (D  9)  PB1  3|    |12  PA1  (D  1)
 *                     PB3  4|    |11  PA2  (D  2)
 *  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3)
 *  PWM        (D  7)  PA7  6|    |9   PA4  (D  4)
 *  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
 *                           +----+
 */
enum PINASSIGN {
  // SPI pins
  MISO = 3,
  MOSI = 0,
  SCK  = 1,
  CS   = 2,
  // Power control
  PWR_SPI = 7,
  PWR_I2C = 8,
  // Serial port
  RX = 10,
  TX = 9,
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

typedef enum {
  SPI_READ  = 0x03, //!< Read data from chip
  SPI_WRITE = 0x02, //!< Write data to chip
  SPI_WREN  = 0x06, //!< Write enable
  SPI_RDSR  = 0x05, //!< Read status register
  } SPI_COMMANDS;

typedef enum {
  SPI_STATUS_WIP = 0x01, //!< Write in progress
  } SPI_STATUS;

//! The line input buffer
static uint8_t s_szLine[LINE_LENGTH];

//! Current mode
static MODE s_mode;

//--- Write buffer management
static uint8_t  s_buffer[BUFFER_SIZE]; //!< The buffer itself
static uint32_t s_buffBase;            //!< Base address of buffer
static uint16_t s_buffIndex;           //!< Current index into buffer

//--- Chip characteristics
static bool     s_spi;          //!< True for SPI interface
static uint16_t s_pageSize;     //!< Size of a page in bytes
static uint8_t  s_addrBytes;    //!< Number of bytes to send as an address
static uint32_t s_chipSize;     //!< Chip capacity in bytes

//---------------------------------------------------------------------------
// Hex conversion helpers
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// Serial communications helpers
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// EEPROM Interface
//---------------------------------------------------------------------------

/** Send a multibyte address over SPI
 */
static void spiSendAddress(uint32_t address) {
  uint32_t mask = 0x000000ffL << ((s_addrBytes - 1) * 8);
  for(uint8_t count=s_addrBytes; count; count--) {
    uint32_t val = (address & mask) >> ((count - 1) * 8);
    shiftOut(MOSI, SCK, MSBFIRST, val);
    mask = mask >> 8;
    }
  }

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
  // Select the chip (assume it is already powered)
  digitalWrite(CS, LOW);
  // Send command and address
  shiftOut(MOSI, SCK, MSBFIRST, SPI_READ);
  spiSendAddress(addr);
  // Now read the data
  for(uint16_t index=0;index<length;index++)
    pBuffer[index] = shiftIn(MISO, SCK, MSBFIRST);
  // Deselect the chip
  digitalWrite(CS, HIGH);
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
  // Enable writes
  digitalWrite(CS, LOW);
  shiftOut(MOSI, SCK, MSBFIRST, SPI_WREN);
  digitalWrite(CS, HIGH);
  // Now write the page
  digitalWrite(CS, LOW);
  shiftOut(MOSI, SCK, MSBFIRST, SPI_WRITE);
  spiSendAddress(addr); 
  for(uint16_t index=0; index<s_pageSize; index++)
    shiftOut(MOSI, SCK, MSBFIRST, pBuffer[index]);
  digitalWrite(CS, HIGH);
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

/** Send a response to the client
 *
 * @param success if true the previous command succeeded.
 * @param cszMessage pointer to a text message (in PROGMEM) to add to the response
 */
void respond(bool success, const char *cszMessage) {
  uartWrite(success?'+':'-');
  if(cszMessage)
    uartPrintP(cszMessage);
  uartWrite(EOL);
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
  uartFormatP(PSTR("+%S %uKb, %u byte page, %u byte address.\n"), s_spi?PSTR("SPI"):PSTR("I2C"), (uint16_t)(s_chipSize >> 10), s_pageSize, s_addrBytes);
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
  s_szLine[length + 4] = (uint8_t)(check >> 8);
  s_szLine[length + 5] = (uint8_t)(check & 0xFF);
  // Send the response
  uartWrite('+');
  for(uint16_t index=0; index<(length + 5); index++)
    uartPrintHex(s_szLine[index + 1], 2);
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
  if(data!=0) {
    respond(false, PSTR("Unexpected data in command."));
    return false;
    }
  // Do we have anything left to write?
  if(s_buffIndex>0) {
    // Grab the data already in the page to fill it out then write
    if(s_spi) {
      spiReadData(s_buffBase + s_buffIndex, s_pageSize - s_buffIndex, &s_buffer[s_buffIndex]);
      spiWritePage(s_buffBase, s_buffer);
      }
    else {
      i2cReadData(s_buffBase + s_buffIndex, s_pageSize - s_buffIndex, &s_buffer[s_buffIndex]);
      i2cWritePage(s_buffBase, s_buffer);
      }
    }
  // All done
  respond(true, NULL);
  return true;
  }

//---------------------------------------------------------------------------
// Main program (setup and loop)
//---------------------------------------------------------------------------

/** Initialisation
 */
void setup() {
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  digitalWrite(SCK, LOW);
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  pinMode(PWR_SPI, OUTPUT);
  digitalWrite(PWR_SPI, LOW);
  pinMode(PWR_I2C, OUTPUT);
  digitalWrite(PWR_I2C, LOW);
  // Disable Timer0 interrupts
  TIMSK0 = 0;
  // Set up serial port
  uartInit();
  // Enter waiting mode
  uartPrintP(BANNER);
  s_mode = MODE_WAITING;
  }

/** Main program loop
 */
void loop() {
  uint8_t data = readLine();
  if(data==0xFF) // Invalid line
    respond(false, PSTR("Unrecognised command."));
  else if(s_szLine[0]==CMD_RESET) {
    // Reset module
    s_mode = MODE_WAITING;
    digitalWrite(PWR_SPI, LOW);
    digitalWrite(PWR_I2C, LOW);
    uartPrintP(BANNER);
    }
  else {
    if(s_mode==MODE_WAITING) {
      if(s_szLine[0]==CMD_INIT) {
        // Initialise device
        if(doInit(data)) {
          // Power on the selected device
          digitalWrite(s_spi?PWR_SPI:PWR_I2C, HIGH);
          s_mode = MODE_READY;
          }
        }
      else
        respond(false, PSTR("Command invalid for mode."));
      }
    else if(s_mode==MODE_READY) {
      if(s_szLine[0]==CMD_READ)
        doRead(data);
      else if(s_szLine[0]==CMD_WRITE) {
        if(doWrite(data, true))
          s_mode = MODE_WRITING;
        }
      else
        respond(false, PSTR("Command invalid for mode."));
      }
    else if(s_mode==MODE_WRITING) {
      if(s_szLine[0]==CMD_WRITE)
        doWrite(data, false);
      else if(s_szLine[0]==CMD_DONE) {
        if(doDone(data))
          s_mode = MODE_READY;
        }
      else
        respond(false, PSTR("Command invalid for mode."));
      }
    else
      respond(false, PSTR("Firmware fault, invalid mode."));
    }
  }

