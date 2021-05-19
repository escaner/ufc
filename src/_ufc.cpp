#define DCSBIOS_DEFAULT_SERIAL

#include <stdint.h>
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

static const uint16_t SWITCH_LOOP_CNT = 500U;

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
  {  6,  0 }, // COM2 for some reason pin 7 does not work, using 0 (RXI) instead
  { 16, 10 }, // HDG
  { 15, 14 }, // CRS
};
static const uint8_t PIN_KP[NUM_KP] = { A1, A2, A3 };
static const uint8_t PIN_LED[NUM_LED] = { 5, 4, A0 };
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

// F/A-18C
constexpr uint8_t FA18C_UFCSPSTR_SZ = 2U;
constexpr uint8_t FA18C_UFCSPNUM_SZ = 8U;
constexpr int FA18C_UFCSPSTR1_ADDR = 0x744e;
constexpr int FA18C_UFCSPSTR2_ADDR = 0x7450;
constexpr int FA18C_UFCSPNUM_ADDR = 0x7446;
constexpr int FA18C_UFCOPCUE_SZ = 1U;
constexpr int FA18C_UFCOPCUE1_ADDR = 0x7428;
constexpr int FA18C_UFCOPCUE2_ADDR = 0x742a;
constexpr int FA18C_UFCOPCUE3_ADDR = 0x742c;
constexpr int FA18C_UFCOPCUE4_ADDR = 0x742e;
constexpr int FA18C_UFCOPCUE5_ADDR = 0x7430;
constexpr int FA18C_UFCOPSTR_SZ = 4U;
constexpr int FA18C_UFCOPSTR1_ADDR = 0x7432;
constexpr int FA18C_UFCOPSTR2_ADDR = 0x7436;
constexpr int FA18C_UFCOPSTR3_ADDR = 0x743a;
constexpr int FA18C_UFCOPSTR4_ADDR = 0x743e;
constexpr int FA18C_UFCOPSTR5_ADDR = 0x7442;
constexpr uint8_t FA18C_UFCCOM_SZ = 2U;
constexpr int FA18C_UFCCOM1_ADDR = 0x7424;
constexpr int FA18C_UFCCOM2_ADDR = 0x7426;


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

DcsBios::LED mcLed(0x1012, 0x0800, 13);

/***********/
/* Methods */
/***********/


/*
  case MODE_M2000C:
    Lcd.print("N  45:37.8       P07");
    Lcd.print("PRET   M91  M92  M93");
    Lcd.print("E 135:12.4  55   D03");
    Lcd.print("ALN  MIP  N.DEG  SEC");
    break;
*/


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
 *   Callback to update F/A-18C heading display.
 */
/*
static void cbFa18cHdg(char *szValue)
{
  DiPnl.fa18cHdg(szValue);
}
*/
/*
 *   Callback to update F/A-18C course display.
 */
/*
static void cbFa18cCrs(char *szValue)
{
  DiPnl.fa18cCrs(szValue);
}
*/
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


void setup()
{
  uint8_t ModeKeyId;

  // Initialize display panel
  DiPnl.init();

  // Initialize all input stuff and get mode of operation
  ModeKeyId = SwPnl.init(KP_MODE);
  WorkMode = Mode(ModeKeyId);

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
  case Mode::M_DEBUG:
    modeDebugInit();
    break;
  case Mode::M_FA18C:
    modeFa18cInit();
    break;
  }

#pragma GCC diagnostic pop

  // Prepare DCS-BIOS stuff
  DcsBios::setup();
}


void loop()
{
  // Check buttons
  processInput();

  // Process DCS-BIOS
  DcsBios::loop();
}
