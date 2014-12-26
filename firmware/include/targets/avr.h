/*---------------------------------------------------------------------------*
* Target definitions for AVR processors.
*----------------------------------------------------------------------------*
* 19-Nov-2014 ShaneG
*
*---------------------------------------------------------------------------*/
#ifndef __TARGETS_AVR_H
#define __TARGETS_AVR_H

//--- Required definitions
#include <avr/io.h>

//--- Set limits for this target
#define EEPROM_SIZE 512

//--- Define what we do and don't support
#define TARGET_HAS_I2C
#define TARGET_HAS_SPI
#define TARGET_HAS_SERIAL
#define TARGET_HAS_EEPROM

#ifdef __cplusplus
extern "C" {
#endif

/** ADC options flags
 */
typedef enum _ADC_OPTIONS {
  ADC_OPTION_NONE = 0x00,
  } ADC_OPTIONS;

/** PWM options flags
 */
typedef enum _PWM_OPTIONS {
  PWM_OPTION_NONE = 0x00,
  } PWM_OPTIONS;

#ifdef __cplusplus
}
#endif

#endif /* __TARGETS_AVR_H */

