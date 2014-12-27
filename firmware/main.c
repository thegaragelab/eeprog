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

/** Program entry point
 */
void main()  {
  hwInit();
  uartPrintP(BANNER);
  while(true) {
    // DEBUG: For now, just echo back to make sure the code is running
    uint8_t ch = uartRead();
    uartWrite(ch);
    }
  }
