#include "displpnl.h"
#include <Arduino.h>


const char DisplPnl::_LINE_KEY[] PROGMEM = "%03u Key:%c%u/%02u Dx:%c%02u";
const char DisplPnl::_LINE_ENC[] PROGMEM = "%03u Enc:%c%u%-3s Dx:%c%02u";


// Constants for position of data in LCD
constexpr uint8_t FA18C_SCRPAD_STR1_COL = 0U;
constexpr uint8_t FA18C_SCRPAD_STR2_COL = 2U;
constexpr uint8_t FA18C_SCRPAD_NUMBER_COL = 4U;
constexpr uint8_t FA18C_SCRPAD_ROW = 0U;
constexpr uint8_t FA18C_OPTION_CUE_COL[] = { 19U, 19U, 19U , 0U, 0U };
constexpr uint8_t FA18C_OPTION_STR_COL[] = { 15U, 15U, 15U , 1U, 1U };
constexpr uint8_t FA18C_OPTION_ROW[] = { 0U, 1U, 2U, 1U, 2U };
constexpr uint8_t FA18C_COM1_COL = 0U;
constexpr uint8_t FA18C_COM2_COL = 18U;
constexpr uint8_t FA18C_COM_ROW = 3U;
/*
constexpr uint8_t FA18C_HDG_TAG_COL = 3U;
constexpr uint8_t FA18C_CRS_TAG_COL = 14U;
constexpr uint8_t FA18C_HDG_COL = 6U;
constexpr uint8_t FA18C_CRS_COL = 11U;
constexpr uint8_t FA18C_HDGCRS_ROW = 3U;
*/


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
  char Line[LCD_COLS+1];
  uint8_t Column;

  // Clear display
  _Lcd.clear();

  // Prepare string with mode and claculate column to center it
  sprintf_P(Line, PSTR("* %S *"), (PGM_P) pmMode);
  Column = (LCD_COLS - (uint8_t) strlen(Line)) / 2;

  // Display the mode
  _Lcd.setCursor(Column, 0);
  _Lcd.print(Line);
}


/*
 *   Initializes display panel for default mode.
 */
void DisplPnl::debugStart()
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
void DisplPnl::debugShowEvent(const Event &Ev, const Directx::Event_t &Dx)
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
 *   Initializes display panel for F/A-18C mode.
 */
void DisplPnl::fa18cStart()
{
  _Lcd.clear();
/*
  _Lcd.setCursor(FA18C_HDG_TAG_COL, FA18C_HDGCRS_ROW);
  _Lcd.print(F("HDG"));
  _Lcd.setCursor(, FA18C_HDGCRS_ROW);
  _Lcd.print(F("CRS"));
*/
}


/*
 *   Updates F/A-18 scratchpad's first string in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadStr1(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_STR1_COL, FA18C_SCRPAD_ROW);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 scratchpad's second string in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadStr2(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_STR2_COL, FA18C_SCRPAD_ROW);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 scratchpad's number in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadNumber(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_NUMBER_COL, FA18C_SCRPAD_ROW);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 option cue in LCD.
 *  Parameters:
 *  * Id [0, 4]: option identifier
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cOptionCue(uint8_t Id, const char *szValue)
{
  _Lcd.setCursor(FA18C_OPTION_CUE_COL[Id], FA18C_OPTION_ROW[Id]);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 option string in LCD.
 *  Parameters:
 *  * Id [0, 4]: option identifier
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cOptionStr(uint8_t Id, const char *szValue)
{
  _Lcd.setCursor(FA18C_OPTION_STR_COL[Id], FA18C_OPTION_ROW[Id]);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 COMM1 channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cCom1(const char *szValue)
{
  _Lcd.setCursor(FA18C_COM1_COL, FA18C_COM_ROW);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 COMM2 channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cCom2(const char *szValue)
{
  _Lcd.setCursor(FA18C_COM2_COL, FA18C_COM_ROW);
  _Lcd.print(szValue);
}


/*
 *   Updates F/A-18 heading in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
/*
void DisplPnl::fa18cHdg(const char *szValue)
{
  _Lcd.setCursor(FA18C_HDG_COL, FA18C_HDGCRS_ROW);
  _Lcd.print(szValue);
}
*/

/*
 *   Updates F/A-18 course in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
/*
void DisplPnl::fa18cCrs(const char *szValue)
{
  _Lcd.setCursor(FA18C_CRS_COL, FA18C_HDGCRS_ROW);
  _Lcd.print(szValue);
}
*/

/*
 *   Shows an error message in the display. PROGMEM friendly version.
 *  Parameters:
 *  * pMsg: string with the error message.
 */
/*
void DisplPnl::error(const __FlashStringHelper *pmMsg)
{
  _error();
  _Lcd.print(pmMsg);
}
*/


/*
 *   Displays Error in the first line of the display and prepares the second
 *  one for the message.
 */
/*
void DisplPnl::_error()
{
  _Lcd.clear();
  _Lcd.print(F("ERROR:"));
  _Lcd.setCursor(0, 1);
}
*/

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
