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

  void f16cStart();
  void f16cDed(uint8_t Line, const char *szValue);
  void f16cFuelQtySelKnob(uint8_t Value);
  void f16cFuelQtyIndicator(bool Fr, uint16_t Value);
  void f16cFuelTotalizerHundreds(uint16_t Value);
  void f16cFuelTotalizerThousands(bool TenK, uint16_t Value);
  void f16cMasterCaut(uint8_t Value);
  void f16cMasterArm(uint8_t Value);
  void f16cStoresCat(uint8_t Value);

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
  void fa18cMasterArm(uint8_t Value);
  void fa18cLtdr(uint8_t Value);

  void m2000cStart();
  void m2000cPcnDigLeft(const char *szValue);
  void m2000cPcnLeft(const char *szValue);
  void m2000cPcnDigRight(const char *szValue);
  void m2000cPcnRight(const char *szValue);
  void m2000cPcnPrep(const char *szValue);
  void m2000cPcnDest(const char *szValue);
  void m2000cPcnMode(uint8_t Value);
  void m2000cPcnButtonLt(uint8_t Value);
  void m2000cPcnPanneLt(uint8_t Value);
  void m2000cPcnMemLt(uint8_t Value);
  void m2000cPanneLt(uint8_t Value);

  void debugStart();
  void debugShowEvent(const Event &Ev, const Directx::Event_t &EvDx);


protected:

  /******************************/
  /* Protected static constants */
  /******************************/

  static const uint8_t _CRD_DIM = 2U; // Dimensions of the LCD coordinates

  static const char _LINE_KEY[] PROGMEM;
  static const char _LINE_ENC[] PROGMEM;

  static const uint8_t _LCD_CHAR_UPDOWN[] PROGMEM;
  static const uint8_t _LCD_CHAR_DELTA[] PROGMEM;
  static const uint8_t _LCD_CHAR_RHO[] PROGMEM;
  static const char _LCD_CHAR_UPDOWN_ID = 1;
  static const char _LCD_CHAR_DELTA_ID = 2;
  static const char _LCD_CHAR_RHO_ID = 3;
  static const char _F16C_CHAR_REPLACEMENT[][2];

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

  static const uint8_t _F16C_FUEL_NUM_POS = 6U;
  static const uint8_t _F16C_FUEL_POS_LN = 4U;
  static const char _F16C_FUEL_POS[_F16C_FUEL_NUM_POS][_F16C_FUEL_POS_LN+1]
    PROGMEM;
  static const uint8_t _STATUS_F16C_SPLINE_NONE = UINT8_MAX;
  static const uint16_t _STATUS_F16C_FUELALFR_NONE = UINT16_MAX;
  static const uint8_t _STATUS_F16C_FUELT_NONE = UINT8_MAX;

  static const char _M2000C_LIGHT_CHAR = '\xff';  // Full block
  static const uint8_t _M2000C_PCNMODE_NUM_POS = 11U;
  static const uint8_t _M2000C_PCNMODE_POS_LN = 5U;
  static const char
    _M2000C_PCNMODE_POS[_M2000C_PCNMODE_NUM_POS][_M2000C_PCNMODE_POS_LN+1]
    PROGMEM;


  /*********************/
  /* Protected methods */
  /*********************/

  void _writeSeparators(const uint8_t (*pmCrd)[_CRD_DIM], uint8_t Size);
  inline static char _f16cReplaceChar(char DedChar);
  void _f16cDedWrite(uint8_t LcdRow, uint8_t LcdCol, const char *sText,
    uint8_t Size);
  void _f16DedUpdateScratchpad(uint8_t Line, const char *szDedText);
  void _f16cDedClearArrows(uint8_t Line);
  void _fa18cFuelWriteSuffix(bool Down, bool SetCursor);
  inline void _m2000cSetButtonLt(uint8_t LcdRow, uint8_t LcdCol, bool Set);
  inline void _m2000cSetPcnLt(uint8_t LcdRow, uint8_t LcdCol, bool Set,
    const char *pmszLabel);
//  void _error();
  inline void _lcdWriteN(uint8_t Count=LCD_COLS, char Char=' ');
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

    // F-16C
    struct
    {
      uint8_t SpLine;   // DED row displayed in the scratchpad
                        // (or _STATUS_F16C_SPLINE_NONE)
      uint8_t SpLength; // Number of chars written in the scratchpad
      uint16_t FuelAl, FuelFr;  // Fuel for AL and FR gauges
      uint8_t FuelTot100, FuelTot1k, FuelTot10k;  // Fuel for total indicator
    } F16c;
  
    // F/A-18C
    struct
    {
      bool IfeiBingo;  // Showing Bingo label
      char Ifei[2];    // Upper/Lower fuel line character
    } Fa18c;

    // Mirage 2000C
    struct
    {
      uint8_t BtnLt;    // PCN Button lights
      uint8_t PanneLt;  // PCN caution & warning lights
      uint8_t MemLt;    // PCN M lights
    } M2000c;
  
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


/*
 *   Writes Cound Chars in the LCD current position. Useful to clear lines,
 *  etc.
 *  Parameters:
 *  * Count: the number of times to write the character.
 *  * Char: the character to write several times.
 */
inline void DisplPnl::_lcdWriteN(uint8_t Count, char Char)
{
  while (Count--)
    _Lcd.write(Char);
}


#endif // _DISPLPNL_H_