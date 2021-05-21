#include "displpnl.h"
#include <Arduino.h>


const char DisplPnl::_LINE_KEY[] PROGMEM = "%03u Key:%c%u/%02u Dx:%c%02u";
const char DisplPnl::_LINE_ENC[] PROGMEM = "%03u Enc:%c%u%-3s Dx:%c%02u";
const char DisplPnl::_LCD_SEPARATOR_CHAR PROGMEM = '|';

// Where to display separators {row, col} format
const uint8_t DisplPnl::_FA18C_SEPARATORS[][2] PROGMEM =
{
  { 0, 11 }, { 0, 14 }, // Row 0
  { 1,  5 }, { 1, 14 },  // Row 1
  { 2,  5 }, { 2, 14 },  // Row 2
  { 3,  2 }, { 3, 17 }   // Row 3
};

/* Constants for position of data in LCD */
// A-10C
constexpr uint8_t A10C_SCRPAD_COL = 1U;
constexpr uint8_t A10C_SCRPAD_ROW = 0U;
constexpr uint8_t A10C_SCRPAD_SZ = 17U;
constexpr uint8_t A10C_HDG_COL = 0U;
constexpr uint8_t A10C_HDG_LBL_COL = 3U;
constexpr uint8_t A10C_CRS_COL = 17U;
constexpr uint8_t A10C_CRS_LBL_COL = 14U;
constexpr uint8_t A10C_HDGCRS_ROW = 3U;
constexpr uint8_t A10C_MASTERARM_ARM_SW = 2U;

// F/A-18C
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
constexpr uint8_t FA18C_FUEL_COL = 7U;
constexpr uint8_t FA18C_FUEL_UP_ROW = 1U;
constexpr uint8_t FA18C_FUEL_DN_ROW = 2U;
constexpr uint8_t FA18C_FUEL_SZ = 6U;
constexpr uint8_t FA18C_BINGO_COL = 10U;
constexpr uint8_t FA18C_BINGO_LBL_COL = 5U;
constexpr uint8_t FA18C_BINGO_ROW = 3U;
constexpr uint8_t FA18C_BINGO_SZ = 5U;


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
  _Lcd.write(Line);
}


/*
 *   Initializes display panel for A-10C mode.
 */
void DisplPnl::a10cStart()
{
  // Clear the display
  _Lcd.clear();

  // Display heading and course labels
  _Lcd.setCursor(A10C_HDG_LBL_COL, A10C_HDGCRS_ROW);
  _Lcd.print(F("HDG"));
  _Lcd.setCursor(A10C_CRS_LBL_COL, A10C_HDGCRS_ROW);
  _Lcd.print(F("CRS"));
}


/*
 *   Updates A-10C scratchpad string in LCD. It may exand through two lines.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cScrpad(const char *szValue)
{
  _Lcd.setCursor(A10C_SCRPAD_COL, A10C_SCRPAD_ROW);
  _Lcd.write(szValue, A10C_SCRPAD_SZ);
}


/*
 *   Updates A-10C selected heading bug value. It is referenced from current
 *  heading.
 *  Parameters:
 *  * Value [0, 65535]: new heading value
 */
void DisplPnl::a10cHdgBug(uint16_t Value)
{
  uint16_t Deg;
  char Buff[3+1];

  // Add to current heading and convert to 360º range
  Deg = (Value - _Status.A10c.Hdg) / (UINT16_MAX / 360U);
 
  _Lcd.setCursor(A10C_HDG_COL, A10C_HDGCRS_ROW);
  sprintf_P(Buff, PSTR("%03u"), Deg);
  _Lcd.write(Buff);
}


/*
 *   Updates A-10C selected course value. It is referenced from current heading.
 *  Parameters:
 *  * Value  [0, 65535]: new heading value
 */
void DisplPnl::a10cCrs(uint16_t Value)
{
  uint16_t Deg;
  char Buff[3+1];

  // Add to current heading and convert to 360º range
  Deg = (Value - _Status.A10c.Hdg) / (UINT16_MAX / 360U);
  // Sometimes 360º just shows up due to round errors, just
  if (Deg == 360U)
    Deg = 0U;

  _Lcd.setCursor(A10C_CRS_COL, A10C_HDGCRS_ROW);
  sprintf_P(Buff, PSTR("%03u"), Deg);
  _Lcd.write(Buff);
}


/*
 *   Updates A-10C master caution light LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::a10cMasterCaut(uint8_t Value)
{
  _setLed(LedWrn, Value);
}


/*
 *   Updates A-10C master arm light LED.
 *  Parameters:
 *  * Value: 0 train, 1 safe, 2 arm
 */
void DisplPnl::a10cMasterArm(uint8_t Value)
{
  _setLed(LedClr, Value==A10C_MASTERARM_ARM_SW);
}


/*
 *   Updates A-10C Gun Ready light LED.
 *  Parameters:
 *  * Value: (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::a10cGunReady(uint8_t Value)
{
  _setLed(LedEnt, Value);
}


/*
 *   Initializes display panel for F/A-18C mode.
 */
void DisplPnl::fa18cStart()
{
  constexpr uint8_t Max =
    sizeof _FA18C_SEPARATORS / sizeof _FA18C_SEPARATORS[0];
  uint8_t Idx;
  char Separator = (char) pgm_read_byte(&_LCD_SEPARATOR_CHAR);

  // Clear the display
  _Lcd.clear();

  // Print the field separators
  for (Idx=0U; Idx<Max; Idx++)
  {
    _Lcd.setCursor(
      pgm_read_byte(_FA18C_SEPARATORS[Idx]+1),
      pgm_read_byte(_FA18C_SEPARATORS[Idx]));
    _Lcd.write(Separator);
  }
}


/*
 *   Updates F/A-18C scratchpad's first string in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadStr1(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_STR1_COL, FA18C_SCRPAD_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C scratchpad's second string in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadStr2(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_STR2_COL, FA18C_SCRPAD_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C scratchpad's number in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cScrpadNumber(const char *szValue)
{
  _Lcd.setCursor(FA18C_SCRPAD_NUMBER_COL, FA18C_SCRPAD_ROW);
  _Lcd.write(szValue + 1);  // Start on second character
}


/*
 *   Updates F/A-18C option cue in LCD.
 *  Parameters:
 *  * Id [0, 4]: option identifier
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cOptionCue(uint8_t Id, const char *szValue)
{
  _Lcd.setCursor(FA18C_OPTION_CUE_COL[Id], FA18C_OPTION_ROW[Id]);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C option string in LCD.
 *  Parameters:
 *  * Id [0, 4]: option identifier
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cOptionStr(uint8_t Id, const char *szValue)
{
  _Lcd.setCursor(FA18C_OPTION_STR_COL[Id], FA18C_OPTION_ROW[Id]);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C COMM1 channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cCom1(const char *szValue)
{
  _Lcd.setCursor(FA18C_COM1_COL, FA18C_COM_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C COMM2 channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::fa18cCom2(const char *szValue)
{
  _Lcd.setCursor(FA18C_COM2_COL, FA18C_COM_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates F/A-18C IFEI upper/lower fuel reading.
 *  Parameters:
 *  * Down: false for upper line, true for lower line.
 *  * szValue: string with the new value to display, padded on the right.
 */
void DisplPnl::fa18cFuel(bool Down, const char *szValue)
{
  char Unpadded[FA18C_FUEL_SZ + 1U];
  uint8_t Len;

  _Lcd.setCursor(FA18C_FUEL_COL, Down? FA18C_FUEL_DN_ROW: FA18C_FUEL_UP_ROW);

  // Remove right padding
  _unpad(Unpadded, szValue);

  // Write with padding on the left
  _lcdWritePadded(Unpadded, FA18C_FUEL_SZ);

  // Calculate last character and save it in _Status
  Len = strlen(Unpadded);
  _Status.Fa18c.Ifei[Down] = Len==0U? '\0': Unpadded[Len-1];

  // Write the L/R suffix
  _fa18cFuelWriteSuffix(Down, false);
}


/*
 *   Updates F/A-18C IFEI BINGO reading.
 *  Parameters:
 *  * szValue: string with the new value to display, padded on the right.
 */
void DisplPnl::fa18cBingo(const char *szValue)
{
  char Unpadded[FA18C_BINGO_SZ + 1U];

  _Lcd.setCursor(FA18C_BINGO_COL, FA18C_BINGO_ROW);

  // Remove right padding
  _unpad(Unpadded, szValue);

  // Write with padding on the left
  _lcdWritePadded(Unpadded, FA18C_BINGO_SZ);
}


/*
 *   Updates F/A-18C IFEI BINGO label.
 *  Parameters:
 *  * Show: true shows the label, false hides it.
 */
void DisplPnl::fa18cBingoLbl(bool Show)
{
  _Lcd.setCursor(FA18C_BINGO_LBL_COL, FA18C_BINGO_ROW);
  // Display or erase label according to value
  _Lcd.print(Show? F("BINGO"): F("     "));

  // Update status
  _Status.Fa18c.IfeiBingo = Show;

  // Write the L/R suffix of both fuel lines
  _fa18cFuelWriteSuffix(false, true);
  _fa18cFuelWriteSuffix(true, true);
}


/*
 *   Updates F/A-18C master caution light LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::fa18cMasterCaut(uint8_t Value)
{
  _setLed(LedWrn, Value);
}


/*
 *   Updates F/A-18C APU Ready light LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
/*
void DisplPnl::fa18cApuReady(uint8_t Value)
{
  _setLed(LedClr, Value);
}
*/

/*
 *   Updates F/A-18C Master Arm LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::fa18cMasterArm(uint8_t Value)
{
  _setLed(LedClr, Value);
}


/*
 *   Updates F/A-18C Master Arm LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::fa18cLtdr(uint8_t Value)
{
  _setLed(LedEnt, Value);
}


/*
 *   Initializes display panel for debug mode.
 */
void DisplPnl::debugStart()
{
  _Lcd.clear();
  _Status.Debug.Line = 0U;
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
    sprintf_P(Buffer, _LINE_KEY, _Status.Debug.Line, PRESS, Ev.Kp.KpId, Ev.Kp.KeyId,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvKpRelease:
    sprintf_P(Buffer, _LINE_KEY, _Status.Debug.Line, RELEASE, Ev.Kp.KpId, Ev.Kp.KeyId,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCcwPress:
    sprintf_P(Buffer, _LINE_ENC, _Status.Debug.Line, PRESS, Ev.EncId, CCW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCcwRelease:
    sprintf_P(Buffer, _LINE_ENC, _Status.Debug.Line, RELEASE, Ev.EncId, CCW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCwPress:
    sprintf_P(Buffer, _LINE_ENC, _Status.Debug.Line, PRESS, Ev.EncId, CW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  case Event::EvEncCwRelease:
    sprintf_P(Buffer, _LINE_ENC, _Status.Debug.Line, RELEASE, Ev.EncId, CW,
      Dx.Action == Directx::AcRelease? RELEASE: PRESS, Dx.Button);
    break;
  }

#pragma GCC diagnostic pop

  _Lcd.setCursor(0, _Status.Debug.Line % LCD_ROWS);
  _Lcd.write(Buffer);

  _Status.Debug.Line++;
}


/*
 *   Initializes display panel for Mirage 2000C mode.
 */
/*
void DisplPnl::m2000cStart()
{
  _Lcd.home();
  _Lcd.write("N  45:37.8       P07");
  _Lcd.write("E 135:12.4  55   D03");
  _Lcd.write("PRET   M91  M92  M93");
  _Lcd.write("ALN  MIP  N.DEG  SEC");
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
  _Lcd.write(pmMsg);
}
*/


/*
 *   Display L or R after the fuel line when no Bingo label and the character
 *  is not blank or 'C' (center).
 *   Parameters:
 *   * Down: wether this is Up or Down fuel line
 *   * SetCursor: whether a setCursor() call is needed to position the cursor
 *     in the LCD.
 */
void DisplPnl::_fa18cFuelWriteSuffix(bool Down, bool SetCursor)
{
  char Suffix;
  char LastChar;  // Last char in the fuel line we are calculating the suffix

  // Display L or R when no Bingo label and the character is not blank or 'C'
  if (!_Status.Fa18c.IfeiBingo && (LastChar = _Status.Fa18c.Ifei[Down])!='\0' &&
      LastChar!='C')
    Suffix = Down? 'r': 'l';
  else
    Suffix = ' ';

  // Do we need to position the cursor before writing the suffix?
  if (SetCursor)
    _Lcd.setCursor(FA18C_FUEL_COL + FA18C_FUEL_SZ,
      Down? FA18C_FUEL_DN_ROW: FA18C_FUEL_UP_ROW);

  // Write the suffix char
  _Lcd.write(Suffix);
}


/*
 *   Displays Error in the first line of the display and prepares the second
 *  one for the message.
 */
/*
void DisplPnl::_error()
{
  _Lcd.clear();
  _Lcd.write(F("ERROR:"));
  _Lcd.setCursor(0, 1);
}
*/


/*
 *   Given a string and a field Size, writes the sting into the current
 *  position in the _Lcd with left white padding to adjust to the field size.
 *   If the string is longer than Size, the result is undefined.
 *   Parameters:
 *   * szText: the string to write
 *   * Size: field size
 */
void DisplPnl::_lcdWritePadded(const char *szText, uint8_t Size)
{
  uint8_t Whites = Size - (uint8_t) strlen(szText);

  // Write blank padding on the left
  while (Whites--)
    _Lcd.write(' ');

  // Write the field value
  _Lcd.write(szText);
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


/*
 *   Removes padding from a string copying to another buffer.
 *  Parameters:
 *  * szDst: destination string (unpadded)
 *  * szSrc: source string (possibly padded)
 */
void DisplPnl::_unpad(char *szDst, const char *szSrc)
{
  // Traverse the whole source string
  for ( ; *szSrc != '\0'; szSrc++)
    // Copy char if not blank
    if (*szSrc != ' ')
      *szDst++ = *szSrc;

  // End destination string
  *szDst = '\0';
}
