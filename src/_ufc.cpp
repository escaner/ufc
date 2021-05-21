#define DCSBIOS_DEFAULT_SERIAL

#include <stdint.h>
#include <EEPROM.h>
#include <Joystick.h>
#include <DcsBios.h>
#include "config.h"
#include "mode.h"
#include "switchpnl.h"
#include "displpnl.h"
#include "directx.h"


/*******************/
/* Local constants */
/*******************/

// Number of iterations processing switches per loop() call
static const uint16_t SWITCH_LOOP_CNT = 150U;

// EEPROM address where the default mode is stored
static const int EEPROM_MODE_ADDR PROGMEM = 0;


/** PRO MICRO pin connections **/

constexpr uint8_t PIN_LCD_EN = 2U;
constexpr uint8_t PIN_LCD_RW = 1U;
constexpr uint8_t PIN_LCD_RS = 0U;
constexpr uint8_t PIN_LCD_D4 = 4U;
constexpr uint8_t PIN_LCD_D5 = 5U;
constexpr uint8_t PIN_LCD_D6 = 6U;
constexpr uint8_t PIN_LCD_D7 = 7U;
constexpr uint8_t PIN_LCD_BL = 3U;

static const uint8_t PIN_ENCODER[NUM_ENC][ENC_NUM_PINS] =
{
  {  8,  9 }, // COM1
  {  6,  0 }, // COM2 pin 7 does not work in my Pro Micro, using 0 (RXI) instead
  { 16, 10 }, // HDG
  { 15, 14 }, // CRS
};
static const uint8_t PIN_KP[NUM_KP] = { A1, A2, A3 };
static const uint8_t PIN_LED[NUM_LED] = { 4, 5, A0 };
// SDA: D2
// MCL: D3


/** Keypad constants **/

// Keypad where to read the mode at boot time
static const uint8_t KP_MODE = 0;


/** LCD **/

constexpr uint8_t LCD_ADDR = 0x27;

static const DisplPnl::LcdData_t LcdData =
{
  LCD_ADDR,
  PIN_LCD_EN,
  PIN_LCD_RW,
  PIN_LCD_RS,
  PIN_LCD_D4,
  PIN_LCD_D5,
  PIN_LCD_D6,
  PIN_LCD_D7,
  PIN_LCD_BL,
  POSITIVE
};


/* DCS-BIOS */

// A-10C
constexpr uint8_t A10C_CDUSP_SZ = 24U;
constexpr unsigned int A10C_CDUSP_ADDR = 0x1298;
constexpr unsigned int A10C_MASTERCAUTLT_ADDR = 0x1012;
constexpr unsigned int A10C_MASTERCAUTLT_MASK = 0x0800;
constexpr unsigned char A10C_MASTERCAUTLT_SHIFT = 11U;
constexpr unsigned int A10C_HSIHDG_ADDR = 0x104c;
constexpr unsigned int A10C_HSIHDG_MASK = 0xffff;
constexpr unsigned char A10C_HSIHDG_SHIFT = 0U;
constexpr unsigned int A10C_HSIHDGBUG_ADDR = 0x1052;
constexpr unsigned int A10C_HSIHDGBUG_MASK = 0xffff;
constexpr unsigned char A10C_HSIHDGBUG_SHIFT = 0U;
constexpr unsigned int A10C_HSICRS_ADDR = 0x1054;
constexpr unsigned int A10C_HSICRS_MASK = 0xffff;
constexpr unsigned char A10C_HSICRS_SHIFT = 0U;
constexpr unsigned int A10C_MASTERARMSW_ADDR = 0x10e8;
constexpr unsigned int A10C_MASTERARMSW_MASK = 0x000c;
constexpr unsigned char A10C_MASTERARMSW_SHIFT = 2U;
constexpr unsigned int A10C_GUNRDYLT_ADDR = 0x1026;
constexpr unsigned int A10C_GUNRDYLT_MASK = 0x8000;
constexpr unsigned char A10C_GUNRDYLT_SHIFT = 15U;

// F/A-18C
constexpr uint8_t FA18C_UFCSPSTR_SZ = 2U;
constexpr uint8_t FA18C_UFCSPNUM_SZ = 8U;
constexpr unsigned int FA18C_UFCSPSTR1_ADDR = 0x744e;
constexpr unsigned int FA18C_UFCSPSTR2_ADDR = 0x7450;
constexpr unsigned int FA18C_UFCSPNUM_ADDR = 0x7446;
constexpr uint8_t FA18C_UFCOPCUE_SZ = 1U;
constexpr unsigned int FA18C_UFCOPCUE1_ADDR = 0x7428;
constexpr unsigned int FA18C_UFCOPCUE2_ADDR = 0x742a;
constexpr unsigned int FA18C_UFCOPCUE3_ADDR = 0x742c;
constexpr unsigned int FA18C_UFCOPCUE4_ADDR = 0x742e;
constexpr unsigned int FA18C_UFCOPCUE5_ADDR = 0x7430;
constexpr uint8_t FA18C_UFCOPSTR_SZ = 4U;
constexpr unsigned int FA18C_UFCOPSTR1_ADDR = 0x7432;
constexpr unsigned int FA18C_UFCOPSTR2_ADDR = 0x7436;
constexpr unsigned int FA18C_UFCOPSTR3_ADDR = 0x743a;
constexpr unsigned int FA18C_UFCOPSTR4_ADDR = 0x743e;
constexpr unsigned int FA18C_UFCOPSTR5_ADDR = 0x7442;
constexpr uint8_t FA18C_UFCCOM_SZ = 2U;
constexpr unsigned int FA18C_UFCCOM1_ADDR = 0x7424;
constexpr unsigned int FA18C_UFCCOM2_ADDR = 0x7426;
constexpr uint8_t FA18C_IFEIFUEL_SZ = 6U;
constexpr unsigned int FA18C_IFEIFUELUP_ADDR = 0x748a;
constexpr unsigned int FA18C_IFEIFUELDN_ADDR = 0x7484;
constexpr uint8_t FA18C_IFEIBINGO_SZ = 5U;
constexpr unsigned int FA18C_IFEIBINGO_ADDR = 0x7462;
constexpr uint8_t FA18C_IFEIBINGOT_SZ = 1U;
constexpr unsigned int FA18C_IFEIBINGOT_ADDR = 0x74b6;
constexpr unsigned int FA18C_MASTERCAUTLT_ADDR = 0x7408;
constexpr unsigned int FA18C_MASTERCAUTLT_MASK = 0x0200;
constexpr unsigned char FA18C_MASTERCAUTLT_SHIFT = 9U;
/*
constexpr unsigned int FA18C_APURDYLT_ADDR = 0x74bc;
constexpr unsigned int FA18C_APURDYLT_MASK = 0x0400;
constexpr unsigned char FA18C_APURDYLT_SHIFT = 10U;
*/
constexpr unsigned int FA18C_MASTERARMSW_ADDR = 0x740c;
constexpr unsigned int FA18C_MASTERARMSW_MASK = 0x2000;
constexpr unsigned char FA18C_MASTERARMSW_SHIFT = 13U;
constexpr unsigned int FA18C_LTDRSW_ADDR = 0x74c4;
constexpr unsigned int FA18C_LTDRSW_MASK = 0x0100;
constexpr unsigned char FA18C_LTDRSW_SHIFT = 8U;


/*************/
/* Variables */
/*************/

static Mode WorkMode;

// Switch panel: reads buttons (keypads) and rotary encoders
static SwitchPnl SwPnl(PIN_KP, PIN_ENCODER);

// LCD
static DisplPnl DiPnl(LcdData, PIN_LED);

// Joystick emulation object to send DirecX button events
static Joystick_ Joy(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  KP0_NUM_KEYS + KP1_NUM_KEYS + KP2_NUM_KEYS + NUM_ENC*2, 0,
  false, false, false, false, false, false, false, false, false, false, false);


/***********/
/* Methods */
/***********/

/* DCS-BIOS callbacks for A-10C */

/*
 *   Callback to update A-10C scratchpad.
 */
static void cbA10cCduScrpad(char *szValue)
{
  DiPnl.a10cScrpad(szValue);
}


/*
 *   Callback to update A-10C HSI heading.
 *  Parameters:
 *  * Value: [0, 65535]
 */
static void cbA10cHsiHdg(unsigned int Value)
{
  DiPnl.a10cHdg(Value);
}

/*
 *   Callback to update A-10C HSI heading bug.
 *  Parameters:
 *  * Value: [0, 65535]
 */
static void cbA10cHsiHdgBug(unsigned int Value)
{
  DiPnl.a10cHdgBug(Value);
}

/*
 *   Callback to update A-10C HSI heading bug.
 *  Parameters:
 *  * Value: [0, 65535]
 */
static void cbA10cHsiCrs(unsigned int Value)
{
  DiPnl.a10cCrs(Value);
}

/*
 *   Callback to update A-10C Master Caution light.
 */
static void cbA10cMasterCautLt(unsigned int Value)
{
  DiPnl.a10cMasterCaut((uint8_t) Value);
}

/*
 *   Callback to update A-10C Master Arm switch position.
 */
static void cbA10cMasterArmSw(unsigned int Value)
{
  DiPnl.a10cMasterArm((uint8_t) Value);
}

/*
 *   Callback to update A-10C Gun Ready light.
 */
static void cbA10cGunReadyLt(unsigned int Value)
{
  DiPnl.a10cGunReady((uint8_t) Value);
}

/*
 *   Initializes A-10C mode.
 */
static void modeA10cInit()
{
  // Initializes display
  DiPnl.a10cStart();

  // Register callbacks creating DCS-BIOS handlers in heap memory

  // CDU Scratchpad
  new DcsBios::StringBuffer<A10C_CDUSP_SZ>(
      A10C_CDUSP_ADDR, cbA10cCduScrpad);
/*
  // COMM
  new DcsBios::StringBuffer<FA18C_UFCCOM_SZ>(
      FA18C_UFCCOM1_ADDR, cbFa18cUfcCom1);
  new DcsBios::StringBuffer<FA18C_UFCCOM_SZ>(
      FA18C_UFCCOM2_ADDR, cbFa18cUfcCom2);
*/
  // Heading and course
  new DcsBios::IntegerBuffer(A10C_HSIHDG_ADDR, A10C_HSIHDG_MASK,
      A10C_HSIHDG_SHIFT, cbA10cHsiHdg);
  new DcsBios::IntegerBuffer(A10C_HSIHDGBUG_ADDR, A10C_HSIHDGBUG_MASK,
      A10C_HSIHDGBUG_SHIFT, cbA10cHsiHdgBug);
  new DcsBios::IntegerBuffer(A10C_HSICRS_ADDR, A10C_HSICRS_MASK,
      A10C_HSICRS_SHIFT, cbA10cHsiCrs);

  // Master warning light
  new DcsBios::IntegerBuffer(A10C_MASTERCAUTLT_ADDR, A10C_MASTERCAUTLT_MASK,
      A10C_MASTERCAUTLT_SHIFT, cbA10cMasterCautLt);
  new DcsBios::IntegerBuffer(A10C_MASTERARMSW_ADDR, A10C_MASTERARMSW_MASK,
      A10C_MASTERARMSW_SHIFT, cbA10cMasterArmSw);
  new DcsBios::IntegerBuffer(A10C_GUNRDYLT_ADDR, A10C_GUNRDYLT_MASK,
      A10C_GUNRDYLT_SHIFT, cbA10cGunReadyLt);
}

/* DCS-BIOS callbacks for F/A-18C */

/*
 *   Callback to update F/A-18C scratchpad's string 1.
 */
static void cbFa18cUfcScrpadStr1(char *szValue)
{
  DiPnl.fa18cScrpadStr1(szValue);
}

/*
 *   Callback to update F/A-18C scratchpad's string 2.
 */
static void cbFa18cUfcScrpadStr2(char *szValue)
{
  DiPnl.fa18cScrpadStr2(szValue);
}

/*
 *   Callback to update F/A-18C scratchpad's number.
 */
static void cbFa18cUfcScrpadNumber(char *szValue)
{
  DiPnl.fa18cScrpadNumber(szValue);
}

/*
 *   Callback to update F/A-18C option cueing 1.
 */
static void cbFa18cUfcOptionCue1(char *szValue)
{
  DiPnl.fa18cOptionCue(0U, szValue);
}

/*
 *   Callback to update F/A-18C option cueing 2.
 */
static void cbFa18cUfcOptionCue2(char *szValue)
{
  DiPnl.fa18cOptionCue(1U, szValue);
}

/*
 *   Callback to update F/A-18C option cueing 3.
 */
static void cbFa18cUfcOptionCue3(char *szValue)
{
  DiPnl.fa18cOptionCue(2U, szValue);
}

/*
 *   Callback to update F/A-18C option cueing 4.
 */
static void cbFa18cUfcOptionCue4(char *szValue)
{
  DiPnl.fa18cOptionCue(3U, szValue);
}

/*
 *   Callback to update F/A-18C option cueing 5.
 */
static void cbFa18cUfcOptionCue5(char *szValue)
{
  DiPnl.fa18cOptionCue(4U, szValue);
}

/*
 *   Callback to update F/A-18C option string 1.
 */
static void cbFa18cUfcOptionStr1(char *szValue)
{
  DiPnl.fa18cOptionStr(0U, szValue);
}

/*
 *   Callback to update F/A-18C option string 2.
 */
static void cbFa18cUfcOptionStr2(char *szValue)
{
  DiPnl.fa18cOptionStr(1U, szValue);
}

/*
 *   Callback to update F/A-18C option string 3.
 */
static void cbFa18cUfcOptionStr3(char *szValue)
{
  DiPnl.fa18cOptionStr(2U, szValue);
}

/*
 *   Callback to update F/A-18C option string 4.
 */
static void cbFa18cUfcOptionStr4(char *szValue)
{
  DiPnl.fa18cOptionStr(3U, szValue);
}

/*
 *   Callback to update F/A-18C option string 5.
 */
static void cbFa18cUfcOptionStr5(char *szValue)
{
  DiPnl.fa18cOptionStr(4U, szValue);
}

/*
 *   Callback to update F/A-18 COM1 display.
 */
static void cbFa18cUfcCom1(char *szValue)
{
  DiPnl.fa18cCom1(szValue);
}

/*
 *   Callback to update F/A-18C COM2 display.
 */
static void cbFa18cUfcCom2(char *szValue)
{
  DiPnl.fa18cCom2(szValue);
}

/*
 *   Callback to update F/A-18C upper line fuel display.
 */
static void cbFa18cIfeiFuelUp(char *szValue)
{
  DiPnl.fa18cFuel(false, szValue);
}

/*
 *   Callback to update F/A-18C lower line fuel display.
 */
static void cbFa18cIfeiFuelDn(char *szValue)
{
  DiPnl.fa18cFuel(true, szValue);
}

/*
 *   Callback to update F/A-18C bingo fuel display.
 */
static void cbFa18cIfeiBingo(char *szValue)
{
  DiPnl.fa18cBingo(szValue);
}

/*
 *   Callback to update F/A-18C bingo fuel display.
 */
static void cbFa18cIfeiBingoTexture(char *szValue)
{
  DiPnl.fa18cBingoLbl(*szValue == '1');
}

/*
 *   Callback to update F/A-18C Master Caution light.
 */
static void cbFa18cMasterCautLt(unsigned int Value)
{
  DiPnl.fa18cMasterCaut((uint8_t) Value);
}

/*
 *   Callback to update F/A-18C APU Ready light.
 */
/*
static void cbFa18cApuReadyLt(unsigned int Value)
{
  DiPnl.fa18cApuReady((uint8_t) Value);
}
*/

/*
 *   Callback on change of F/A-18C Master Arm switch.
 */
static void cbFa18cMasterArmSw(unsigned int Value)
{
  DiPnl.fa18cMasterArm((uint8_t) Value);
}

/*
 *   Callback on change of F/A-18C LTD/R switch.
 */
static void cbFa18cLtdrSw(unsigned int Value)
{
  DiPnl.fa18cLtdr((uint8_t) Value);
}

/*
 *   Initializes F/A-18C mode.
 */
static void modeFa18cInit()
{
  // Initializes display
  DiPnl.fa18cStart();

  // Register callbacks creating DCS-BIOS handlers in heap memory

  // Scratchpad
  new DcsBios::StringBuffer<FA18C_UFCSPSTR_SZ>(
      FA18C_UFCSPSTR1_ADDR, cbFa18cUfcScrpadStr1);
  new DcsBios::StringBuffer<FA18C_UFCSPSTR_SZ>(
      FA18C_UFCSPSTR2_ADDR, cbFa18cUfcScrpadStr2);
  new DcsBios::StringBuffer<FA18C_UFCSPNUM_SZ>(
      FA18C_UFCSPNUM_ADDR, cbFa18cUfcScrpadNumber);

  // Option cueing
  new DcsBios::StringBuffer<FA18C_UFCOPCUE_SZ>(
      FA18C_UFCOPCUE1_ADDR, cbFa18cUfcOptionCue1);
  new DcsBios::StringBuffer<FA18C_UFCOPCUE_SZ>(
      FA18C_UFCOPCUE2_ADDR, cbFa18cUfcOptionCue2);
  new DcsBios::StringBuffer<FA18C_UFCOPCUE_SZ>(
      FA18C_UFCOPCUE3_ADDR, cbFa18cUfcOptionCue3);
  new DcsBios::StringBuffer<FA18C_UFCOPCUE_SZ>(
      FA18C_UFCOPCUE4_ADDR, cbFa18cUfcOptionCue4);
  new DcsBios::StringBuffer<FA18C_UFCOPCUE_SZ>(
      FA18C_UFCOPCUE5_ADDR, cbFa18cUfcOptionCue5);

  // Option string
  new DcsBios::StringBuffer<FA18C_UFCOPSTR_SZ>(
      FA18C_UFCOPSTR1_ADDR, cbFa18cUfcOptionStr1);
  new DcsBios::StringBuffer<FA18C_UFCOPSTR_SZ>(
      FA18C_UFCOPSTR2_ADDR, cbFa18cUfcOptionStr2);
  new DcsBios::StringBuffer<FA18C_UFCOPSTR_SZ>(
      FA18C_UFCOPSTR3_ADDR, cbFa18cUfcOptionStr3);
  new DcsBios::StringBuffer<FA18C_UFCOPSTR_SZ>(
      FA18C_UFCOPSTR4_ADDR, cbFa18cUfcOptionStr4);
  new DcsBios::StringBuffer<FA18C_UFCOPSTR_SZ>(
      FA18C_UFCOPSTR5_ADDR, cbFa18cUfcOptionStr5);

  // COMM
  new DcsBios::StringBuffer<FA18C_UFCCOM_SZ>(
      FA18C_UFCCOM1_ADDR, cbFa18cUfcCom1);
  new DcsBios::StringBuffer<FA18C_UFCCOM_SZ>(
      FA18C_UFCCOM2_ADDR, cbFa18cUfcCom2);

  // IFEI
  // It is important that bingo texture is updated before fuel lines
  new DcsBios::StringBuffer<FA18C_IFEIFUEL_SZ>(
      FA18C_IFEIFUELUP_ADDR, cbFa18cIfeiFuelUp);
  new DcsBios::StringBuffer<FA18C_IFEIFUEL_SZ>(
      FA18C_IFEIFUELDN_ADDR, cbFa18cIfeiFuelDn);
  new DcsBios::StringBuffer<FA18C_IFEIBINGOT_SZ>(
      FA18C_IFEIBINGOT_ADDR, cbFa18cIfeiBingoTexture);
  new DcsBios::StringBuffer<FA18C_IFEIBINGO_SZ>(
      FA18C_IFEIBINGO_ADDR, cbFa18cIfeiBingo);

  // Master warning light
  new DcsBios::IntegerBuffer(FA18C_MASTERCAUTLT_ADDR, FA18C_MASTERCAUTLT_MASK,
      FA18C_MASTERCAUTLT_SHIFT, cbFa18cMasterCautLt);

  // APU Ready light
/*
  new DcsBios::IntegerBuffer(FA18C_APURDYLT_ADDR, FA18C_APURDYLT_MASK,
      FA18C_APURDYLT_SHIFT, cbFa18cApuReadyLt);
*/

  // Master Arm and LTD/R switches
  new DcsBios::IntegerBuffer(FA18C_MASTERARMSW_ADDR, FA18C_MASTERARMSW_MASK,
      FA18C_MASTERARMSW_SHIFT, cbFa18cMasterArmSw);
  new DcsBios::IntegerBuffer(FA18C_LTDRSW_ADDR, FA18C_LTDRSW_MASK,
      FA18C_LTDRSW_SHIFT, cbFa18cLtdrSw);
}


/*
 *   Initializes debug mode
 */
static void modeDebugInit()
{
  DiPnl.debugStart();
}


/*
 *   Checks for input events and handles them.
 */
static void processInput()
{
  Event Ev;
  Directx::Event_t EvDx;

  // Get button event
  Ev = SwPnl.check(SWITCH_LOOP_CNT);

  // Any event received?
  if (Ev.Id != Event::EvNone)
  {
    // Yes, translate to DirectX
    EvDx = Directx::translate(Ev);

    // Send DirectX event to PC
    if (EvDx.Action == Directx::AcRelease)
      Joy.releaseButton(EvDx.Button);
    else
      Joy.pressButton(EvDx.Button);

    // Show stuff in debug mode
    if (WorkMode.get() == Mode::M_DEBUG)
      DiPnl.debugShowEvent(Ev, EvDx);
  }
}


/*
 *   Given a pressed key during boot, initializes WorkMode as the mode of
 *  operation. If no key was pressed, the previous mode is used (stored in
 *  EEPROM). When a key is pressed, the mode is stored in EEPROM for future
 *  use.
 *   Parameters:
 *   * KeyId: key pressed during boot (can also be SwitchKp::SWITCH_NONE) when
 *     no key was pressed.
 */
static void setupWorkMode(uint8_t KeyId)
{
  const int EepromAddr = pgm_read_word(&EEPROM_MODE_ADDR);

  // Did the user manually selected a mode?
  if (KeyId == SwitchKp::SWITCH_NONE)
  {
    // No key pressed, default to previous mode used (stored in EEPROM)
    // Note that if the EEPROM is not initialized and an invalid mode is
    // read, the default mode will be selected and the EEPROM will remain
    // not initialized
    WorkMode.set((Mode::Id_t) EEPROM.read(EepromAddr));
  }
  else
  {
    // User selected a mode, get it and store it in EEPROM
    WorkMode = Mode(KeyId);
    EEPROM.update(EepromAddr, WorkMode.get());
  }
}


/*
 *   Code entry point: initializes all the stuff.
 */
void setup()
{
  uint8_t ModeKeyId;

  // Initialize display panel
  DiPnl.init();

  // Initialize all input stuff and setup mode of operation selected
  ModeKeyId = SwPnl.init(KP_MODE);
  setupWorkMode(ModeKeyId);

  // Display mode of operation
  DiPnl.showMode(WorkMode.P_str());

  // Initialize joystick object
  Joy.begin();

  // Wait for the release of the keypad mode button
  SwPnl.wait(KP_MODE);

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

  // Initialize mode stuff
  switch (WorkMode.get())
  {
  case Mode::M_A10C:
    modeA10cInit();
    break;
  case Mode::M_FA18C:
    modeFa18cInit();
    break;
  case Mode::M_DEBUG:
    modeDebugInit();
    break;
  }

#pragma GCC diagnostic pop

  // Prepare DCS-BIOS stuff
  DcsBios::setup();
}


/*
 *   Main loop function.
 */
void loop()
{
  // Check buttons
  processInput();

  // Process DCS-BIOS
  DcsBios::loop();
}
