/*--------------------------------------------------------------------------*
* Main program
*---------------------------------------------------------------------------*
* 14-Apr-2014 ShaneG
*
* Template program for ATtiny85 C/asm projects.
*--------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <microboard.h>
#include <softuart.h>
#include <mbstring.h>

// Program banner
#define BANNER PSTR("EEPROG V0.1\n")

// Forward definition of 'main()'
void main() __attribute__ ((noreturn));

/** Program entry point
 */
void main()  {
  uartInit();
  uartPrintP(BANNER);
  while(true);
  }
