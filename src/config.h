#ifndef _CONFIG_H_
#define _CONFIG_H_

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

// Delay between DirectX events
constexpr uint8_t DX_DELAY_PR = 5U;  // Delay press -> release (ms)
constexpr uint8_t DX_DELAY_RP = 5U;  // Delay release -> press (ms)
// Special delays for other modes
constexpr uint8_t F16C_ICP_DX_DELAY_PR = 80U;
//constexpr uint8_t F16C_HDGCRS_DX_DELAY_PR = 50U;

// Debounce time for keypads (us)
constexpr unsigned long KP_DEBOUNCE_TIME = 800UL;


#endif  // _CONFIG_H_
