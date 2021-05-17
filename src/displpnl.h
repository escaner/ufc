#ifndef _DISPLPNL_H_
#define _DISPLPNL_H_

#include <stdint.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "event.h"
#include "directx.h"


/*
 *   Class to manage the display panel, comprised by the LCD and the LEDs.
 */
class DisplPnl
{
public:

  /**************/
  /* Data types */
  /**************/

  // LED identifiers to access them
  enum LedId_t
  {
    LedClr = 0,
    LedEnt = 1,
    LedWrn = 2
  };

  // Data to initialize LCD
  struct LcdData_t
  {
    uint8_t Addr;
    uint8_t PinEn;
    uint8_t PinRw;
    uint8_t PinRs;
    uint8_t PinD4;
    uint8_t PinD5;
    uint8_t PinD6;
    uint8_t PinD7;
    uint8_t PinBl;
    t_backlightPol Pol;
  };


  /******************/
  /* Public methods */
  /******************/

  DisplPnl(const LcdData_t &LcdData, const uint8_t LedPin[NUM_LED]);
  void init();
  void showMode(const __FlashStringHelper *pmMode);
  void error(const __FlashStringHelper *pmMsg);
  void defaultStart();
  void defaultShowEvent(const Event &Ev, const Directx::Event_t &EvDx);

protected:

  /*********************/
  /* Protected methods */
  /*********************/

 void _error();
 inline void _setLed(LedId_t Led, uint8_t Value) const;


  /***************/
  /* Member data */
  /***************/

  static const char _LINE_KEY[] PROGMEM;
  static const char _LINE_ENC[] PROGMEM;

  LiquidCrystal_I2C _Lcd;
  const uint8_t _LedPin[NUM_LED];

  uint8_t _defaultLine;  // Current line for logs
};


#endif // _DISPLPNL_H_