#include "displpnl.h"


const char DisplPnl::_LINE_KEY[] PROGMEM = "%03u Key:%c%u/%02u Dx:%c%02u";
const char DisplPnl::_LINE_ENC[] PROGMEM = "%03u Enc:%c%u%-3s Dx:%c%02u";


/*
 *   Constructor.
 *   Parameters:
 *   * LcdData: struct with data to initialize the LCD.
 *   * LedPin: array with the pins of the LEDs.
 */
DisplPnl::DisplPnl(const LcdData_t &LcdData, const uint8_t LedPin[NUM_LED]):
  _Lcd(LcdData.Addr, LcdData.PinEn, LcdData.PinRw, LcdData.PinRs,
    LcdData.PinD4, LcdData.PinD5, LcdData.PinD6, LcdData.PinD7,
    LcdData.PinBl,LcdData.Pol),
  _LedPin{ LedPin[0], LedPin[1], LedPin[2] }
{
/*
_Lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
*/
}


/*
 *   Initialized LCD and LEDs.
 */
void DisplPnl::init()
{
  // Initialize LCD
  _Lcd.begin(LCD_COLS, LCD_ROWS);

  // Initialize LEDs as output pins
  for (uint8_t LedId = 0U; LedId < NUM_LED; LedId++)
    pinMode(_LedPin[LedId], OUTPUT);    
}


/*
 *   Displays the mode of operation.
 *   Parameters:
 *   * pmMode: program memory string with the mode of operation.
 */
void DisplPnl::showMode(const __FlashStringHelper *pmMode)
{
  _Lcd.clear();
  _Lcd.print(pmMode);
}


/*
 *   Initializes display panel for default mode.
 */
void DisplPnl::defaultStart()
{
  _Lcd.clear();
  _defaultLine = 0U;
}


/*
 *   Displays a key or encoder event and its DirectX conversion.
 *   Parameters:
 *   * Ev: descriptor of the keypad or encoder event. EvNone is not valid.
 *   * Dx: descriptor of the DirectX event.
 */
void DisplPnl::defaultShowEvent(const Event &Ev, const Directx::Event_t &Dx)
{
  static const char PRESS = 'P';
  static const char RELEASE = 'R';
  static const char CCW[] = "CCW";
  static const char CW[] = "CW";
  char Buffer[LCD_COLS+1];

#pragma GCC diagnostic push
  // Disable: warning: enumeration value 'EvNone' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

  switch (Ev.Id)
  {
  case Event::EvKpPress:
    sprintf_P(Buffer, _LINE_KEY, _defaultLine, PRESS, Ev.Kp.KpId, Ev.Kp.KeyId,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvKpRelease:
    sprintf_P(Buffer, _LINE_KEY, _defaultLine, RELEASE, Ev.Kp.KpId, Ev.Kp.KeyId,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCcwPress:
    sprintf_P(Buffer, _LINE_ENC, _defaultLine, PRESS, Ev.EncId, CCW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCcwRelease:
    sprintf_P(Buffer, _LINE_ENC, _defaultLine, RELEASE, Ev.EncId, CCW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCwPress:
    sprintf_P(Buffer, _LINE_ENC, _defaultLine, PRESS, Ev.EncId, CW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCwRelease:
    sprintf_P(Buffer, _LINE_ENC, _defaultLine, RELEASE, Ev.EncId, CW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  }

#pragma GCC diagnostic pop

  _Lcd.setCursor(0, _defaultLine % LCD_ROWS);
  _Lcd.print(Buffer);

  _defaultLine++;
}

/*
 *   Shows an error message in the display. PROGMEM friendly version.
 *  Parameters:
 *  * pMsg: string with the error message.
 */
void DisplPnl::error(const __FlashStringHelper *pmMsg)
{
  _error();
  _Lcd.print(pmMsg);
}


/*
 *   Displays Error in the first line of the display and prepares the second
 *  one for the message.
 */
void DisplPnl::_error()
{
  _Lcd.clear();
  _Lcd.print(F("ERROR:"));
  _Lcd.setCursor(0, 1);
}


/*
 *   Turns on or off a LED.
 *   Parameters:
 *   * LedId [0, NUM_LED]: LED on which to perform the operation.
 *   * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
inline void DisplPnl::_setLed(LedId_t LedId, uint8_t Value) const
{
  digitalWrite(_LedPin[LedId], Value);
}
