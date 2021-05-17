#ifndef _SWITCHPNL_H_
#define _SWITCHPNL_H_

#include "config.h"
#include <Arduino.h>
#include <REncoderAsync.h>
#include <SwitchKp.h>
#include "event.h"


/*
 *   Class to manage switch panel inputs to the Arduino microcontroller.
 *  Keypad 0: numeric buttons
 *  Keypad 1: left LSK, COM, HDG, CRS & Warning
 *  Keypad 2: right LSK, Range, Wpt & simple pushbuttons
 */
class SwitchPnl
{
public:

  /********************/
  /* Static constants */
  /********************/

  static const uint8_t ENC_PIN_A = 0;
  static const uint8_t ENC_PIN_B = 1;


  /******************/
  /* Public methods */
  /******************/

  SwitchPnl(const uint8_t KpPin[NUM_KP],
    const uint8_t EncPin[NUM_ENC][ENC_NUM_PINS]);
  uint8_t init(uint8_t Keypad) const;
  void wait(uint8_t Keypad) const;
  Event check(uint16_t LoopCnt = 1U);


protected:

  /********************/
  /* Static constants */
  /********************/

  // Analog value range (min, max) for each button in kp matrix; range [0, 1023]
  static const int16_t _KP0_KEY_RNG[KP0_NUM_KEYS][2];
  static const int16_t _KP1_KEY_RNG[KP1_NUM_KEYS][2];
  static const int16_t _KP2_KEY_RNG[KP2_NUM_KEYS][2];

  // Label to access all KP*_NUM_KEYS through indexes
  static const uint8_t _KP_NUM_KEYS[NUM_KP];

  // Label to access all KP*_KEY_RNG through indexes
  static const int16_t (* const _KP_KEY_RNG[NUM_KP])[2];


  /***********************/
  /* Protected constants */
  /***********************/

  const uint8_t *const _KpPin;  // Pins used by analog keypads
  const uint8_t (* const _EncPin)[ENC_NUM_PINS];  // Pins used by encoders


  /*********************/
  /* Protected methods */
  /*********************/

  inline uint8_t _kpGetKey(uint8_t KpId) const;
  uint8_t _kpValueToKey(uint8_t KpId, int16_t KpVal) const;


  /***************/
  /* Member data */
  /***************/

  // Keypad matrixes for buttons
  SwitchKp _Keypad[NUM_KP];         // Manage keypads
  REncoderAsync _Encoder[NUM_ENC];  // Manage Encoders
};


/*
 *   Reads analog value from keypad and translates it to the key identifier.
 *  Arguments:
 *  * KpId: keypad that needs to be read
 *  Returns: key that is pressed in the keypad
 */
inline uint8_t SwitchPnl::_kpGetKey(uint8_t KpId) const
{
  int16_t KpVal;

  KpVal = analogRead(_KpPin[KpId]);
  return _kpValueToKey(KpId, KpVal);
}


#endif  // _SWITCHPNL_H_