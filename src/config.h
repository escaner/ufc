#ifndef _CONFIG_H_
#define _CONFIG_H_

#define NDEBUG
#define __ASSERT_USE_STDERR

#include <stdint.h>


/*
 *   Define global constants for general program configuration.
 */

// Number of keypads
constexpr uint8_t NUM_KP = 3;

// Number of keys on each keypad
constexpr uint8_t KP0_NUM_KEYS = 16U;
constexpr uint8_t KP1_NUM_KEYS = 11U;
constexpr uint8_t KP2_NUM_KEYS = 12U;

// Number of rotary encoders
constexpr uint8_t NUM_ENC = 4;

// Number of Arduino pins per encoder
constexpr uint8_t ENC_NUM_PINS = 2;

// Number of LEDs as user interface output
constexpr uint8_t NUM_LED = 3;

// Dimensions of LCD
constexpr uint8_t LCD_ROWS =  4U;
constexpr uint8_t LCD_COLS = 20U;


#endif  // _CONFIG_H_
