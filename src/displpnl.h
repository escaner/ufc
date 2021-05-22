#ifndef _DISPLPNL_H_
#define _DISPLPNL_H_

// Lots of warnings from this library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#  include <LiquidCrystal_I2C.h>
#pragma GCC diagnostic pop

#include <stdint.h>
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
//  void error(const __FlashStringHelper *pmMsg);

  void a10cStart();
  void a10cScrpad(const char *szValue);
  void a10cUhfFreq(const char *szValue);
  void a10cUhfMode(uint16_t Value);
  void a10cUhfPreset(const char *szValue);
  void a10cVamFreq(const char *szValue);
  void a10cVamMode(uint16_t Value);
  void a10cVamPreset(const char *szValue);
  void a10cVfmFreq(const char *szValue);
  void a10cVfmMode(uint16_t Value);
  void a10cVfmPreset(const char *szValue);
  void a10cTcnChannel(const char *szValue);
  void a10cTcnMode(uint16_t Value);
  void a10cIlsFreq(const char *szValue);
  inline void a10cHdg(uint16_t Value);
  void a10cHdgBug(uint16_t Value);
  void a10cCrs(uint16_t Value);
  void a10cMasterCaut(uint8_t Value);
  void a10cMasterArm(uint8_t Value);
  void a10cGunReady(uint8_t Value);

  void fa18cStart();
  void fa18cScrpadStr1(const char *szValue);
  void fa18cScrpadStr2(const char *szValue);
  void fa18cScrpadNumber(const char *szValue);
  void fa18cOptionCue(uint8_t Id, const char *szValue);
  void fa18cOptionStr(uint8_t Id, const char *szValue);
  void fa18cCom1(const char *szValue);
  void fa18cCom2(const char *szValue);
  void fa18cFuel(bool Down, const char *szValue);
  void fa18cBingo(const char *szValue);
  void fa18cBingoLbl(bool Show);
  void fa18cMasterCaut(uint8_t Value);
//  void fa18cApuReady(uint8_t Value);
  void fa18cMasterArm(uint8_t Value);
  void fa18cLtdr(uint8_t Value);

  void debugStart();
  void debugShowEvent(const Event &Ev, const Directx::Event_t &EvDx);

//  void m2000cStart();

protected:

  /******************************/
  /* Protected static constants */
  /******************************/

  static const uint8_t _CRD_DIM = 2U; // Dimensions of the LCD coordinates

  static const char _LINE_KEY[] PROGMEM;
  static const char _LINE_ENC[] PROGMEM;

  static const char _LCD_SEPARATOR_CHAR = '|';
  static const uint8_t _A10C_SEPARATORS[][_CRD_DIM] PROGMEM;
  static const uint8_t _FA18C_SEPARATORS[][_CRD_DIM] PROGMEM;

  static const uint8_t _A10C_VHF_NUM_MODES = 4U;
  static const uint8_t _A10C_UHF_NUM_MODES = 3U;
  static const uint8_t _A10C_TCN_NUM_MODES = 5U;
  static const uint8_t _A10C_TCN_MODES_LN = 3U;
  static const char _A10C_VHF_MODES[_A10C_VHF_NUM_MODES] PROGMEM;
  static const char _A10C_UHF_MODES[_A10C_UHF_NUM_MODES] PROGMEM;
  static const char _A10C_TCN_MODES[_A10C_TCN_NUM_MODES][_A10C_TCN_MODES_LN+1]
    PROGMEM;


  /*********************/
  /* Protected methods */
  /*********************/

  void _writeSeparators(const uint8_t (*pmCrd)[_CRD_DIM], uint8_t Size);
  void _fa18cFuelWriteSuffix(bool Down, bool SetCursor);
//  void _error();
  void _lcdWritePadded(const char *szText, uint8_t Size, char PadChar=' ');
  void _lcdWriteDeg(uint16_t Value);
  inline void _setLed(LedId_t Led, uint8_t Value) const;
  static void _unpad(char *szDst, const char *szSrc, uint8_t Discard = 0U);


  /***************/
  /* Member data */
  /***************/

  LiquidCrystal_I2C _Lcd;
  const uint8_t _LedPin[NUM_LED];

  // Union of modes with status to display stuff
  union
  {
    // A-10C
    struct
    {
      uint16_t Hdg;  // HSI current heading
    } A10c;

    // F/A-18C
    struct
    {
      bool IfeiBingo;  // Showing Bingo label
      char Ifei[2];    // Upper/Lower fuel line character
    } Fa18c;

    // Debug
    struct
    {
      uint8_t Line;  // Current line for logs
    } Debug;
  } _Status;
};


/*
 *   Updates A-10C heading value for reference from heading bug and course.
 *  Parameters:
 *  * Value [0, 65535]: new heading value
 */
inline void DisplPnl::a10cHdg(uint16_t Value)
{
  _Status.A10c.Hdg = Value;
}


#endif // _DISPLPNL_H_