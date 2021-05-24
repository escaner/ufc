#include "displpnl.h"
#include <Arduino.h>

// Rounded division of positive integers
#define DIV_PROUND(n, d) (((n) + (d)/2)/(d))


/*************/
/* Constants */
/*************/

// Custom character with a double triangle up & down
const uint8_t DisplPnl::_LCD_CHAR_UPDOWN[] PROGMEM =
{
	0b00100,
	0b01110,
	0b11111,
	0b00000,
	0b00000,
	0b11111,
	0b01110,
	0b00100
};

// Some chars from the F-16C DED need to be replaced for correct visualization
const char DisplPnl::_F16C_CHAR_REPLACEMENT[][2] =
{
  { 'a', _LCD_CHAR_UPDOWN_ID },
  { 'o', '\xdf' }  // 'º' sign in LCD display charset
};


const char DisplPnl::_LINE_KEY[] PROGMEM = "%03u Key:%c%u/%02u Dx:%c%02u";
const char DisplPnl::_LINE_ENC[] PROGMEM = "%03u Enc:%c%u%-3s Dx:%c%02u";

// Strings for the radio modes
const char DisplPnl::_A10C_VHF_MODES[_A10C_VHF_NUM_MODES] PROGMEM =
  { 'F', 'A', 'M', 'P' };
const char DisplPnl::_A10C_UHF_MODES[_A10C_UHF_NUM_MODES] PROGMEM =
  { 'M', 'P', 'G' };
const char DisplPnl::_A10C_TCN_MODES[_A10C_TCN_NUM_MODES][_A10C_TCN_MODES_LN+1]
  PROGMEM = { "OFF", "RCV", "T/R", "A-R", "A-T" };

const char DisplPnl::_F16C_FUEL_POS[_F16C_FUEL_NUM_POS][_F16C_FUEL_POS_LN+1]
  PROGMEM = { "TEST", "NRML", "RSVR", "IWNG", "XWNG", "XCTR" };

// Where to display separators {row, col} format
const uint8_t DisplPnl::_A10C_SEPARATORS[][_CRD_DIM] PROGMEM =
{
  { 1, 9 }, { 1, 10 },  // Row 1
  { 2, 9 }, { 2, 10 },  // Row 2
  { 3, 6 }, { 3, 13 }   // Row 3
};

const uint8_t DisplPnl::_FA18C_SEPARATORS[][_CRD_DIM] PROGMEM =
{
  { 0, 11 }, { 0, 14 },  // Row 0
  { 1,  5 }, { 1, 14 },  // Row 1
  { 2,  5 }, { 2, 14 },  // Row 2
  { 3,  2 }, { 3, 17 }   // Row 3
};


/* F-16C DED patterns to recognize page and data */
constexpr char F16C_DED_EDIT_CHAR = '*';
constexpr char F16C_DED_UHF_PTRN[] PROGMEM = "UHF";
constexpr uint8_t F16C_DED_UHF_PTRN_COL = 1U;
constexpr uint8_t F16C_DED_UHF_COL = 5U;
constexpr uint8_t F16C_DED_UHF_SZ = 7U;
constexpr uint8_t F16C_DED_STPT_COL = 19U;
constexpr uint8_t F16C_DED_STPT_SZ = 4U;
constexpr char F16C_DED_VHF_PTRN[] PROGMEM = "VHF";
constexpr uint8_t F16C_DED_VHF_PTRN_COL = 1U;
constexpr uint8_t F16C_DED_VHF_COL = 5U;
constexpr uint8_t F16C_DED_VHF_SZ = 7U;
constexpr char F16C_DED_TCN_PTRN[] PROGMEM = " T";
constexpr uint8_t F16C_DED_TCN_PTRN_COL = 18U;
constexpr uint8_t F16C_DED_TCN_COL = 20U;
constexpr uint8_t F16C_DED_TCN_SZ = 4U;


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
constexpr uint8_t A10C_UHFFREQ_COL = 0U;
constexpr uint8_t A10C_UHFMODE_COL = 6U;
constexpr uint8_t A10C_UHFPSET_COL = 7U;
constexpr uint8_t A10C_UHF_ROW = 2U;
constexpr uint8_t A10C_VAMFREQ_COL = 0U;
constexpr uint8_t A10C_VAMFREQ_SZ = 7U;
constexpr uint8_t A10C_VAMMODE_COL = 6U;
constexpr uint8_t A10C_VAMPSET_COL = 7U;
constexpr uint8_t A10C_VAMPSET_SZ = 2U;
constexpr uint8_t A10C_VAM_ROW = 1U;
constexpr uint8_t A10C_VHFPSET_SZ = 2U;
constexpr uint8_t A10C_VFMFREQ_COL = 14U;
constexpr uint8_t A10C_VFMFREQ_SZ = 7U;
constexpr uint8_t A10C_VFMMODE_COL = 13U;
constexpr uint8_t A10C_VFMPSET_COL = 11U;
constexpr uint8_t A10C_VFM_ROW = 1U;
constexpr uint8_t A10C_TCNFREQ_COL = 16U;
constexpr uint8_t A10C_TCNMODE_COL = 12U;
constexpr uint8_t A10C_TCN_ROW = 2U;
constexpr uint8_t A10C_ILSFREQ_COL = 7U;
constexpr uint8_t A10C_ILS_ROW = 3U;

// F-16C
constexpr uint8_t F16C_UHF_LBL_COL = 0U;
constexpr uint8_t F16C_UHF_COL = 3U;
constexpr uint8_t F16C_STPT_LBL_COL = 12U;
constexpr uint8_t F16C_STPT_COL = 16U;
constexpr uint8_t F16C_UHFSTPT_ROW = 0U;
constexpr uint8_t F16C_VHF_LBL_COL = 0U;
constexpr uint8_t F16C_VHF_COL = 3U;
constexpr uint8_t F16C_VHFTCN_ROW = 1U;
constexpr uint8_t F16C_TCN_LBL_COL = 13U;
constexpr uint8_t F16C_TCN_COL = 16U;
constexpr uint8_t F16C_FUELPOS_COL = 0U;
constexpr uint8_t F16C_FUELAL_COL = 4U;
constexpr uint8_t F16C_FUELSEP_LBL_COL = 8U;
constexpr uint8_t F16C_FUELFR_COL = 9U;
constexpr uint8_t F16C_FUELT_LBL_COL = 14U;
constexpr uint8_t F16C_FUELT_COL = 15U;
constexpr uint8_t F16C_FUEL_ROW = 2U;
constexpr uint8_t F16C_SCRATCHPAD_ROW = 4U;
constexpr uint8_t F16C_MASTERARM_ARM_SW = 2U;
constexpr uint8_t F16C_STORES_CATI_SW = 1U;

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


/***********/
/* Methods */
/***********/

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
  Column = (LCD_COLS - (uint8_t) strlen(Line)) / 2U;

  // Display the mode
  _Lcd.setCursor(Column, 0U);
  _Lcd.write(Line);
}


/*
 *   Initializes display panel for A-10C mode.
 */
void DisplPnl::a10cStart()
{
  constexpr uint8_t Size =
    sizeof _A10C_SEPARATORS / sizeof _A10C_SEPARATORS[0];

  // Clear the display
  _Lcd.clear();

  // Print the field separators
  _writeSeparators(_A10C_SEPARATORS, Size);

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
  // Write only the scratchpad area, not the page at the end of the string
  _Lcd.write(szValue, A10C_SCRPAD_SZ);
}

/*
 *   Updates A-10C UHF radio frequency in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cUhfFreq(const char *szValue)
{
  _Lcd.setCursor(A10C_UHFFREQ_COL, A10C_UHF_ROW);
  // Discard non significant last character in 25kHz separation frequency
  _Lcd.write(szValue, A10C_VAMFREQ_SZ - 1U);
}


/*
 *   Updates A-10C UHF radio mode in LCD.
 *  Parameters:
 *  * Value: mode identifier.
 */
void DisplPnl::a10cUhfMode(uint16_t Value)
{
  _Lcd.setCursor(A10C_UHFMODE_COL, A10C_UHF_ROW);
  _Lcd.write(pgm_read_byte(_A10C_UHF_MODES + Value));
}


/*
 *   Updates A-10C UHF radio preset channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cUhfPreset(const char *szValue)
{
  _Lcd.setCursor(A10C_UHFPSET_COL, A10C_UHF_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates A-10C VHF AM radio frequency in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cVamFreq(const char *szValue)
{
  char Unpadded[A10C_VAMFREQ_SZ + 1U];

  _Lcd.setCursor(A10C_VAMFREQ_COL, A10C_VAM_ROW);
  // Remove right padding and discard last character in 25kHz separation freq
  _unpad(Unpadded, szValue, 1U);
  // Repad with zeroes to the left
  _lcdWritePadded(Unpadded, A10C_VAMFREQ_SZ - 1U, '0');
}


/*
 *   Updates A-10C VHF AM radio mode in LCD.
 *  Parameters:
 *  * Value: mode identifier.
 */
void DisplPnl::a10cVamMode(uint16_t Value)
{
  _Lcd.setCursor(A10C_VAMMODE_COL, A10C_VAM_ROW);
  _Lcd.write(pgm_read_byte(_A10C_VHF_MODES + Value));
}


/*
 *   Updates A-10C VHF AM radio preset channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cVamPreset(const char *szValue)
{
  char Unpadded[A10C_VHFPSET_SZ + 1U];

  _Lcd.setCursor(A10C_VAMPSET_COL, A10C_VAM_ROW);
  // Remove left blank padding
  _unpad(Unpadded, szValue);
  // Repad with zeroes to the left
  _lcdWritePadded(Unpadded, A10C_VAMPSET_SZ, '0');
}


/*
 *   Updates A-10C VHF FM radio frequency in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cVfmFreq(const char *szValue)
{
  char Unpadded[A10C_VFMFREQ_SZ + 1U];

  _Lcd.setCursor(A10C_VFMFREQ_COL, A10C_VFM_ROW);
  // Remove right padding and discard last character in 25kHz separation freq
  _unpad(Unpadded, szValue, 1U);
  // Repad with zeroes to the left
  _lcdWritePadded(Unpadded, A10C_VFMFREQ_SZ - 1U, '0');
}


/*
 *   Updates A-10C VHF FM radio mode in LCD.
 *  Parameters:
 *  * Value: mode identifier.
 */
void DisplPnl::a10cVfmMode(uint16_t Value)
{
  _Lcd.setCursor(A10C_VFMMODE_COL, A10C_VFM_ROW);
  _Lcd.write(pgm_read_byte(_A10C_VHF_MODES + Value));
}


/*
 *   Updates A-10C VHF FM radio preset channel in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cVfmPreset(const char *szValue)
{
  char Unpadded[A10C_VHFPSET_SZ + 1U];

  _Lcd.setCursor(A10C_VFMPSET_COL, A10C_VFM_ROW);
  // Remove left blank padding
  _unpad(Unpadded, szValue);
  // Repad with zeroes to the left
  _lcdWritePadded(Unpadded, A10C_VHFPSET_SZ, '0');
}


/*
 *   Updates A-10C TACAN radio frequency in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cTcnChannel(const char *szValue)
{
  _Lcd.setCursor(A10C_TCNFREQ_COL, A10C_TCN_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates A-10C TACAN radio mode in LCD.
 *  Parameters:
 *  * Value: mode identifier.
 */
void DisplPnl::a10cTcnMode(uint16_t Value)
{
  _Lcd.setCursor(A10C_TCNMODE_COL, A10C_TCN_ROW);
  _Lcd.print((const __FlashStringHelper *) (_A10C_TCN_MODES + Value));
}


/*
 *   Updates A-10C ILS radio frequency in LCD.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::a10cIlsFreq(const char *szValue)
{
  _Lcd.setCursor(A10C_ILSFREQ_COL, A10C_ILS_ROW);
  _Lcd.write(szValue);
}


/*
 *   Updates A-10C selected heading bug value. It is referenced from current
 *  heading.
 *  Parameters:
 *  * Value [0, 65535]: new heading value
 */
void DisplPnl::a10cHdgBug(uint16_t Value)
{
  _Lcd.setCursor(A10C_HDG_COL, A10C_HDGCRS_ROW);
  _lcdWriteDeg(Value);
}


/*
 *   Updates A-10C selected course value. It is referenced from current heading.
 *  Parameters:
 *  * Value  [0, 65535]: new course value
 */
void DisplPnl::a10cCrs(uint16_t Value)
{
  _Lcd.setCursor(A10C_CRS_COL, A10C_HDGCRS_ROW);
  _lcdWriteDeg(Value);
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
 *   Initializes display panel for F-16C mode.
 */
void DisplPnl::f16cStart()
{
//  uint8_t Buffer[sizeof _LCD_CHAR_UPDOWN];
  constexpr char FUEL_ZERO = '0';

  // Initialize status data
  _Status.F16c.SpLine = _STATUS_F16C_SPLINE_NONE;  // Nothing in the scratchpad
  _Status.F16c.FuelAl =
  _Status.F16c.FuelFr = _STATUS_F16C_FUELALFR_NONE;
  _Status.F16c.FuelTot100 =
  _Status.F16c.FuelTot1k =
  _Status.F16c.FuelTot10k = _STATUS_F16C_FUELT_NONE;

  // Clear the display
  _Lcd.clear();

  // Register custom LCD character
//  memcpy_P(Buffer, _LCD_CHAR_UPDOWN, sizeof _LCD_CHAR_UPDOWN);
//  _Lcd.createChar(_LCD_CHAR_UPDOWN_ID, Buffer);
  _Lcd.createChar(_LCD_CHAR_UPDOWN_ID, (PGM_P) _LCD_CHAR_UPDOWN);

  // Display labels
  _Lcd.setCursor(F16C_UHF_LBL_COL, F16C_UHFSTPT_ROW);
  _Lcd.print(F("UHF"));
  _Lcd.setCursor(F16C_STPT_LBL_COL, F16C_UHFSTPT_ROW);
  _Lcd.print(F("STPT"));
  _Lcd.setCursor(F16C_UHF_LBL_COL, F16C_VHFTCN_ROW);
  _Lcd.print(F("VHF"));
  _Lcd.setCursor(F16C_TCN_LBL_COL, F16C_VHFTCN_ROW);
  _Lcd.write("TCN");
  _Lcd.setCursor(F16C_FUELSEP_LBL_COL, F16C_FUEL_ROW);
  _Lcd.write('/');
  _Lcd.setCursor(F16C_FUELT_LBL_COL, F16C_FUEL_ROW);
  _Lcd.write('T');

  // Zero values as last digit for fuel indicators
  _Lcd.setCursor(F16C_FUELAL_COL+3, F16C_FUEL_ROW);
  _Lcd.write(FUEL_ZERO);
  _Lcd.setCursor(F16C_FUELFR_COL+3, F16C_FUEL_ROW);
  _Lcd.write(FUEL_ZERO);
  _Lcd.setCursor(F16C_FUELT_COL+4, F16C_FUEL_ROW);
  _Lcd.write(FUEL_ZERO);
}


/*
 *   Updates LCD with F-16C's DED lines. Looks for entries surrounded by
 *  asterisks and shows them as scratchpad.
 *  Parameters:
 *  * szValue: string with the new value to display.
 */
void DisplPnl::f16cDed(uint8_t Line, const char *szValue)
{
  bool Updated = false;

  // Check for main CNI page values
  switch (Line)
  {
  case 0:  // UHF & STPT
    // Check whether we are on the CNI page
    if (!strncmp_P(szValue + F16C_DED_UHF_PTRN_COL, F16C_DED_UHF_PTRN,
        sizeof F16C_DED_UHF_PTRN - 1U))
    {
      // Copy UHF frequency
      _f16cWriteDed(F16C_UHFSTPT_ROW, F16C_UHF_COL,
        szValue + F16C_DED_UHF_COL, F16C_DED_UHF_SZ);
      // Copy Steerpoint
      _f16cWriteDed(F16C_UHFSTPT_ROW, F16C_STPT_COL,
        szValue + F16C_DED_STPT_COL, F16C_DED_STPT_SZ);
      Updated = true;
    }
    break;
  case 2:  // VHF
    // Check whether we are on the CNI page
    if (!strncmp_P(szValue + F16C_DED_VHF_PTRN_COL, F16C_DED_VHF_PTRN,
        sizeof F16C_DED_VHF_PTRN - 1U))
    {
      // Copy VHF frequency
      _f16cWriteDed(F16C_VHFTCN_ROW, F16C_VHF_COL,
        szValue + F16C_DED_VHF_COL, F16C_DED_VHF_SZ);
      Updated = true;
    }
    break;
  case 4:  // TACAN
    // Check whether we are on the CNI page
    if (!strncmp_P(szValue + F16C_DED_TCN_PTRN_COL, F16C_DED_TCN_PTRN,
        sizeof F16C_DED_TCN_PTRN - 1U))
    {
      // Copy TACAN channel
      _f16cWriteDed(F16C_VHFTCN_ROW, F16C_TCN_COL,
        szValue + F16C_DED_TCN_COL, F16C_DED_TCN_SZ);
      Updated = true;
    }
    break;
  }

  // Check for editedtion fields and display at scratchpad or erase it as needed
  _f16DedUpdateScratchpad(Line, szValue);
}


/*
 *   Updates F-16C fuel quantity selector knob value on the LCD.
 *  Parameters:
 *  * Value: new position for the knob
 */
void DisplPnl::f16cFuelQtySelKnob(uint8_t Value)
{
  _Lcd.setCursor(F16C_FUELPOS_COL, F16C_FUEL_ROW);
  _Lcd.print((const __FlashStringHelper *) (_F16C_FUEL_POS + Value));
}


/*
 *   Updates F-16C fuel quantity AL and FR indicators on the LCD.
 *  The actual fuel value ranges [0, 5000] but it is represented in the range
 *  [0, UINT16_MAX]. This method produces a value in the range [0, 500] so the
 *  last digit of the normalized value will be fixed to 0.
 *   Parameters:
 *   * Fr: true for FR, false for AL indicator
 *   * Value [0, UINT16_MAX]: new value of the reading, needs to be normalized
 *   *                        to [0, 5000].
 */
void DisplPnl::f16cFuelQtyIndicator(bool Fr, uint16_t Value)
{
  // Denominator to normalize to [0, 500]
  constexpr uint32_t DENOMINATOR = (uint32_t) UINT16_MAX * (uint32_t) 10UL;
  constexpr uint32_t MAX_NORM_VALUE = 5000UL;
  char Buff[3+1];
  uint8_t Col;
  bool WriteLcd;
  uint32_t TmpValue;
  uint16_t NormValue; // Value normalized to range [0, 500]

  // Position the cursor according to the indicator
  Col = Fr? F16C_FUELFR_COL: F16C_FUELAL_COL;
  _Lcd.setCursor(Col, F16C_FUEL_ROW);

  // Normalize value
  TmpValue = (uint32_t) Value * MAX_NORM_VALUE;
  NormValue = (uint16_t) DIV_PROUND(TmpValue, DENOMINATOR);

  // Check if the value to display has changed from the previous one
  // and update when so for future reference
  if (Fr)
  {
    if (WriteLcd = NormValue!=_Status.F16c.FuelFr)
      _Status.F16c.FuelFr = NormValue;
  }
  else  // Al
    if (WriteLcd = NormValue!=_Status.F16c.FuelAl)
      _Status.F16c.FuelAl = NormValue;

  // If changed, we need to convert NormValue to string and write it on the LCD
  if (WriteLcd)
  {
    sprintf_P(Buff, PSTR("%03u"), (unsigned) NormValue);
    _Lcd.write(Buff);
  }
}


/*
 *   Updates F-16C fuel totalizer hundreds counter indicator on the LCD.
 *  The actual fuel value ranges [0, 1000) but it is represented in the range
 *  [0, UINT16_MAX]. This method produces a value in the range [0, 99] so the
 *  last digit of the normalized value will be fixed to 0.
 *   Note that the value is truncated and not rounded to tenths. E.g. a value of
 *  997 will be displayed as 990 and not 000. Otherwise, as the thousands are
 *  calculated in another place, if the fuel were 2997 we would be showing 2000
 *  instead of 3000.
 *   Parameters:
 *   * Value [0, UINT16_MAX]: new value of the reading, needs to be normalized
 *                            to [0, 99].
 */
void DisplPnl::f16cFuelTotalizerHundreds(uint16_t Value)
{
  // Denominator to normalize to [0, 100]
  constexpr uint32_t DENOMINATOR = (uint32_t) UINT16_MAX * (uint32_t) 10UL;
  constexpr uint32_t MAX_NORM_VALUE = 1000UL;
  char Buff[2+1];
  uint32_t TmpValue;
  uint8_t NormValue; // Value normalized to range [0, 99]

  // Position the cursor
  _Lcd.setCursor(F16C_FUELT_COL+2, F16C_FUEL_ROW);

  // Normalize value
  TmpValue = (uint32_t) Value * MAX_NORM_VALUE;
  // Truncate: when value is 2,997, show e.g. 2,990 instead of 2,000!
  NormValue = (uint8_t) (TmpValue / DENOMINATOR);  // Truncate!!!

  // We obtain a NormValue [0, 100], make simple modulo for [0, 99]
  if (NormValue == (uint8_t) (MAX_NORM_VALUE / 10UL))
    NormValue = 0U;

  // Check if the value to display has changed from the previous one
  if (NormValue != _Status.F16c.FuelTot100)
  {
    // Update value in _Status for future reference
    _Status.F16c.FuelTot100 = NormValue;

    // Convert to string and write the value
    sprintf_P(Buff, PSTR("%02u"), (unsigned) NormValue);
    _Lcd.write(Buff);
  }
}


/*
 *   Updates F-16C fuel totalizer 1k & 10k counter indicator on the LCD.
 *  The actual fuel value ranges [0, 10) but it is represented in the range
 *  [0, UINT16_MAX]. The key values for the digits are:
 *    Value = (trunc) Digit * UINT16_SIZE / 10
 *  This means that for odd digits we are losing 0.5 precision.
 *  Also, the Value decreases to the lower Digit only for a short time after
 *  after the lower value digit has decreased, it is not linear.
 *   Therefore we will need some tweaking to restore the actual Digit
 *  represented by the Value.
 *   Parameters:
 *   * TenK: true for 10k digit; false for 1k digit.
 *   * Value [0, UINT16_MAX]: new value of the reading, needs to be normalized
 *                            to [0, 10).
 */
void DisplPnl::f16cFuelTotalizerThousands(bool TenK, uint16_t Value)
{
  char Buff[1+1];
  uint8_t Col;
  bool WriteLcd;
  uint32_t TmpDigit;
  uint8_t Digit; // Value normalized to range [0, 9]

  // Position the cursor according to the digit
  Col = F16C_FUELT_COL;
  if (!TenK)
    Col++;
  _Lcd.setCursor(Col, F16C_FUEL_ROW);

  // Calculate integer divisor of Value*10 to avoid decimal errors. We get a
  // candidate Digit, but due to truncation calculating Value it could be
  // an unit too low.
  TmpDigit = (uint32_t) Value * (uint32_t) 10UL / UINT16_MAX;
  Digit = (uint8_t) TmpDigit;

  // We get a value [0, 10], need to quick modulo when 10
  if (Digit == (uint8_t) 10U)
    // If we got Digit==10, no need to check for Digit+1, the result is integer
    // Also, modulo it to 0
    Digit = 0U;
  else  // Not 10 (max 9) -> no overflow in uint16 comparison
    // Check if the next Digit would return the exact same Value after truncation
    if ((uint16_t) ((TmpDigit + (uint32_t) 1UL) * UINT16_MAX / (uint32_t) 10UL)
        == Value)
      // Yes, our digit was one unit too low
      Digit++;  // This cannot deliver 10, no need for modulo

  // Check if the value to display has changed from the previous one
  // and update when so for future reference
  if (TenK)  // 10K digit
  {
    if (WriteLcd = Digit!=_Status.F16c.FuelTot10k)
      _Status.F16c.FuelTot10k = Digit;
  }
  else  // 1K digit
    if (WriteLcd = Digit!=_Status.F16c.FuelTot1k)
      _Status.F16c.FuelTot1k = Digit;

  // If changed, we need to convert Digit to string and write it on the LCD
  if (WriteLcd)
  {
    sprintf_P(Buff, PSTR("%01u"), (unsigned) Digit);
    _Lcd.write(Buff);
  }
}


/*
 *   Updates F-16C master caution light LED.
 *  Parameters:
 *  * Value (HIGH, LOW): new state value: HIGH = on; LOW = off
 */
void DisplPnl::f16cMasterCaut(uint8_t Value)
{
  _setLed(LedWrn, Value);
}


/*
 *   Updates F-16C Master Arm LED. Light is only with switch in ARM position.
 *  Parameters:
 *  * Value (0: Sim, 1: Off, 2: Arm): switch position
 */
void DisplPnl::f16cMasterArm(uint8_t Value)
{
  _setLed(LedClr, Value==F16C_MASTERARM_ARM_SW);
}


/*
 *   Updates F-16C Stores Category LED. Light is turned on for CATI and off
 *   for CATIII.
 *  Parameters:
 *  * Value (0: CATIII, 1: CATI): switch position
 */
void DisplPnl::f16cStoresCat(uint8_t Value)
{
  _setLed(LedEnt, Value==F16C_STORES_CATI_SW);
}


/*
 *   Initializes display panel for F/A-18C mode.
 */
void DisplPnl::fa18cStart()
{
  constexpr uint8_t Size =
    sizeof _FA18C_SEPARATORS / sizeof _FA18C_SEPARATORS[0];

  // Clear the display
  _Lcd.clear();

  // Print the field separators
  _writeSeparators(_FA18C_SEPARATORS, Size);
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
 *   Replace a DED character by its apropriate representation as defined in
 *  _F16C_CHAR_REPLACEMENT array.
 *   Parameters:
 *   * DedChar: the character to check for replacement
 *   Returns: a character replacing DedChar according to the translation table
 *            or DedChar itself if none matches.
 */
inline char DisplPnl::_f16cReplaceChar(char DedChar)
{
  constexpr uint8_t Size =
    sizeof _F16C_CHAR_REPLACEMENT / sizeof _F16C_CHAR_REPLACEMENT[0];
  uint8_t Idx;

  // Look for DedChar on the translation table
  for (Idx = 0U; Idx < Size; Idx++)
    if (DedChar == _F16C_CHAR_REPLACEMENT[Idx][0])
      // Found it! Return its replacemente character
      return _F16C_CHAR_REPLACEMENT[Idx][1];

  // Default action: return the same character
  return DedChar;
}


/*
 *   Copies a string from szText from the DED into the LCD indicated position.
    Special DED characters are reinterpreted.
    Parameters:
    * LcdRow: row in the LCD where to copy the text
    * LcdCol: column in the LCD where to copy the text
    * sText: text to be copìed
    * Size: number of characters from sText to copy
 */
void DisplPnl::_f16cWriteDed(uint8_t LcdRow, uint8_t LcdCol, const char *sText,
  uint8_t Size)
{
  char WriteChar;

  _Lcd.setCursor(LcdCol, LcdRow);

  // Copy as many chars as Size
  while (Size--)
  {
    // Replace char if matching an special one
    WriteChar = _f16cReplaceChar(*sText);

    // Wirte current char into LCD
    _Lcd.write(WriteChar);

    // Advance to next char position
    sText++;
  }
}


/*
 *   Checks whether the szDedText contains an editable field and displays it
 *  on the LCD scratchpad. If none is found, checks of the scratchpad has
 *  data from this very same line and if so deletes it.
 *   Parameters:
 *   * Line: line in the DED that we are processing
 *   * szDedText: text of the DED line
 */
void DisplPnl::_f16DedUpdateScratchpad(uint8_t Line, const char *szDedText)
{
  const char *pBegin, *pEnd;
  uint8_t Length;  // Size of the scratchpad line that we are writing

  // Look for the editable field characters
  if ((pBegin = strchr(szDedText, F16C_DED_EDIT_CHAR)) &&
      (pEnd = strchr(pBegin + 1, F16C_DED_EDIT_CHAR)))
  {
    // Editable field found: copy it to the LCD scratchpad
    Length = (uint8_t) (pEnd - pBegin + 1);
//    _Lcd.setCursor(0U, F16C_SCRATCHPAD_ROW);
//    _Lcd.write(pBegin, Length);
    _f16cWriteDed(F16C_SCRATCHPAD_ROW, 0U, pBegin, Length);

    // If the scratchpad was already displaying text, blank out the rest
    if (_Status.F16c.SpLine!=_STATUS_F16C_SPLINE_NONE &&
        Length<_Status.F16c.SpLength)
      _lcdWriteN(_Status.F16c.SpLength - Length);

    // Update status of the scratchpad
    _Status.F16c.SpLine = Line;
    _Status.F16c.SpLength = Length;
  }
  else if (_Status.F16c.SpLine == Line)
  {
    // No editable field found but the LCD scratchpad was visualizing a field
    // from this line

    // Clear the LCD scratchpad line
    _Lcd.setCursor(0U, F16C_SCRATCHPAD_ROW);
    _lcdWriteN(_Status.F16c.SpLength);

    // Update status: scratchpad is now blank
    _Status.F16c.SpLine = _STATUS_F16C_SPLINE_NONE;
  }
}


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
 *   Writes field separators gettinf the coordinates from paCoord, which is
 *  a two dimensional array stored in program memory.
 *   Parameters:
 *   * pmCrd: two dimensional array with coordinates [row, col], stored in
 *     program memory.
 *   * Size: number of coordinates in pmCrd array
 */
void DisplPnl::_writeSeparators(const uint8_t (*pmCrd)[_CRD_DIM], uint8_t Size)
{
  uint8_t Idx;

  for (Idx=0U; Idx<Size; Idx++)
  {
    // Fetch coordinates from program memory
    _Lcd.setCursor(pgm_read_byte(pmCrd[Idx]+1), pgm_read_byte(pmCrd[Idx]));
    _Lcd.write(_LCD_SEPARATOR_CHAR);
  }
}


/*
 *   Given a string and a field Size, writes the sting into the current
 *  position in the _Lcd with left white padding to adjust to the field size.
 *   If the string is longer than Size, the result is undefined.
 *   Parameters:
 *   * szText: the string to write
 *   * Size: field size
 *   * PadChar: character with which the padding will be filled
 */
void DisplPnl::_lcdWritePadded(const char *szText, uint8_t Size, char PadChar)
{
  uint8_t Whites = Size - (uint8_t) strlen(szText);

  // Write blank padding on the left
  while (Whites--)
    _Lcd.write(PadChar);

  // Write the field value
  _Lcd.write(szText);
}


/*
 *   Converts a Value from uint16_t range to 360º range and writes it into
 *  the LCD.
 *   Parameters:
 *   * Value: the value to convert to degrees
 */
void DisplPnl::_lcdWriteDeg(uint16_t Value)
{
  // Denominator to normalize to [0, 360]
  constexpr uint32_t DENOMINATOR = UINT16_MAX;
  constexpr uint32_t MAX_NORM_VALUE = 360UL;
  char Buff[3+1];
  uint32_t AbsValue;  // Value unreferenced from present heading
  uint16_t Deg;

  // Decouple Value from current heading
  AbsValue = (uint32_t) (Value - _Status.A10c.Hdg);

  // Normalize value with 32 bit precision to avoid overflow
  AbsValue *= MAX_NORM_VALUE;
  Deg = (uint16_t) DIV_PROUND(AbsValue, DENOMINATOR);

  // We obtain a Deg value [0, 360], make simple modulo for [0, 359]
  if (Deg == (uint16_t) MAX_NORM_VALUE)
    Deg = 0U;

  // Convert to string and write on LCD
  sprintf_P(Buff, PSTR("%03u"), (unsigned) Deg);
  _Lcd.write(Buff);
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
 *   Removes padding from a string copying to another buffer and possibly
 *  discarding the last Discard significant chars in the string.
 *  Parameters:
 *  * szDst: destination string (unpadded)
 *  * szSrc: source string (possibly padded)
 *  * Discard: discard last characters
 */
void DisplPnl::_unpad(char *szDst, const char *szSrc, uint8_t Discard)
{
  // Traverse the whole source string
  for ( ; *szSrc != '\0'; szSrc++)
    // Copy char if not blank
    if (*szSrc != ' ')
      *szDst++ = *szSrc;

  // End destination string
  *(szDst - Discard) = '\0';
}
