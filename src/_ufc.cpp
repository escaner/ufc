//#define NDEBUG
//#define DEBUG_SERIAL

#include <stdint.h>
#include <Joystick.h>
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
  {  6,  7 }, // COM2
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


/*
  case MODE_M2000C:
    Lcd.print("N  45:37.8       P07");
    Lcd.print("PRET   M91  M92  M93");
    Lcd.print("E 135:12.4  55   D03");
    Lcd.print("ALN  MIP  N.DEG  SEC");
    break;
  }
*/


void ProcessInput()
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

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

    // Do more stuff depending on mode
    switch (WorkMode.get())
    {
    case Mode::M_DEBUG:
      DiPnl.defaultShowEvent(Ev, EvDx);
      break;
    }

#pragma GCC diagnostic pop

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
  DiPnl.defaultStart();
}


void loop()
{
  ProcessInput();
}
