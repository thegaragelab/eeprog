/*---------------------------------------------------------------------------*
* Digital IO helpers for AVR
*----------------------------------------------------------------------------*
* 27-Dec-2014 ShaneG
*
* Provides the basic digital pin interface. Uses DIP format pin numbers.
*---------------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <microboard.h>

//---------------------------------------------------------------------------
// Pin interface definitions
//---------------------------------------------------------------------------

/** Port for the pin
 */
typedef enum {
  NONE,
  A,
  B,
  } PORT;

//! Define pin information
#define PINDEF(port, bit) (((port)<<4)|(bit))

//! Get the port number from a pin definition
#define GETPORT(def) (((def)>>4)&0x0F)

//! Get the pin numer from a pin definition
#define GETPIN(def) ((def)&0x0F)

#if defined(__AVR_ATtiny84__)
#  define MAX_PIN 14

/** Pin mappings for ATtiny84
 */
static const uint8_t s_pins[] PROGMEM = {
  PINDEF(NONE, 0), //!< 1 = Vcc
  PINDEF(B, 0),    //!< 2 = PB0
  PINDEF(B, 1),    //!< 3 = PB1
  PINDEF(NONE, 0), //!< 4 = RESET
  PINDEF(B, 2),    //!< 5 = PB2
  PINDEF(A, 7),    //!< 6 = PA7
  PINDEF(A, 6),    //!< 7 = PA6
  PINDEF(A, 5),    //!< 8 = PA5
  PINDEF(A, 4),    //!< 9 = PA4
  PINDEF(A, 3),    //!< 10 = PA3
  PINDEF(A, 2),    //!< 11 = PA2
  PINDEF(A, 1),    //!< 12 = PA1
  PINDEF(A, 0),    //!< 13 = PA0
  PINDEF(NONE, 0), //!< 14 = GND
  };
#else
#  error "Unsupported CPU"
#endif

/** Helper to get a pin definition
 *
 * @param pin the pin number to check
 *
 * @return zero if the pin is invalid (out of range or not usable) or the
 *         pin definition code if it is available
 */
static uint8_t isPinValid(uint8_t pin) {
  // Make sure the pin is in range
  if(pin>MAX_PIN)
    return 0;
  return flashReadByte((uint16_t)s_pins + pin);
  }

//---------------------------------------------------------------------------
// Public API
//---------------------------------------------------------------------------

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
bool pinConfig(PIN pin, PIN_DIRECTION direction) {
  // Get the definition
  uint8_t def = isPinValid(pin);
  if(!def)
    return false;
  uint8_t bit = 1 << GETPIN(def);
  // Set up the pin
  if(GETPORT(def)==A) {
    if(direction==OUTPUT)
      DDRA |= bit;
    else {
      DDRA &= ~bit;
      if(direction==INPUT_PULLUP)
        PORTA |= bit;
      }
    }
  else { // PORTB
    if(direction==OUTPUT)
      DDRB |= bit;
    else {
      DDRB &= ~bit;
      if(direction==INPUT_PULLUP)
        PORTB |= bit;
      }
    }
  return true;
  }

/** Change the state of an output pin
 *
 * @param pin the number of the pin to modify.
 * @param state the new state of the pin - true for on (high), false for off (low)
 */
void pinWrite(PIN pin, bool state) {
  // Get the definition
  uint8_t def = isPinValid(pin);
  if(!def)
    return;
  uint8_t bit = 1 << GETPIN(def);
  // Change the output state
  if(GETPORT(def)==A) {
    if(state)
      PINA |= bit;
    else
      PINA &= ~bit;
    }
  else { // PORTB
    if(state)
      PINB |= bit;
    else
      PINB &= ~bit;
    }
  }

/** Read the state of an input pin
 *
 * Read the input of a digital pin. This function assumes the pin has been
 * configured as a digital input - if it has not the results are undefined.
 *
 * @param pin the number of the pin to read.
 *
 * @return true if the input is high, false if the input is low.
 */
bool pinRead(PIN pin) {
  // Get the definition
  uint8_t def = isPinValid(pin);
  if(!def)
    return false;
  uint8_t bit = 1 << GETPIN(def);
  // Read the input state
  if(GETPORT(def)==A)
    return PINA & bit;
  // PORTB
  return PINB & bit;
  }

