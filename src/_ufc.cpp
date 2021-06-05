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
  {  8,  9 }, // EncCom1
  {  6,  0 }, // EncCom2 (pin 7 does not work in my Pro Micro -> using 0 (RXI))
  { 16, 10 }, // EncLeft
  { 15, 14 }, // EncRight
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

// Used in DCS BIOS messages in several modules
static const char _BIOS_MSG_HDG[] PROGMEM = { 'H', 'D', 'G' };
static const char _BIOS_MSG_CRS[] PROGMEM = { 'C', 'R', 'S' };
static const char _BIOS_ARG_INC_CHAR = '+';
static const char _BIOS_ARG_DEC_CHAR = '-';

// A-10C
constexpr uint8_t A10C_CDUSP_SZ = 24U;
constexpr unsigned int A10C_CDUSP_ADDR = 0x1298;
constexpr uint8_t A10C_UHFFREQ_SZ = 7U;
constexpr unsigned int A10C_UHFFREQ_ADDR = 0x1180;
constexpr unsigned int A10C_UHFMODE_ADDR = 0x117c;
constexpr unsigned int A10C_UHFMODE_MASK = 0x0003;
constexpr unsigned int A10C_UHFMODE_SHIFT = 0U;
constexpr uint8_t A10C_UHFPSET_SZ = 2U;
constexpr unsigned int A10C_UHFPSET_ADDR = 0x1188;
constexpr uint8_t A10C_VAMFREQ_SZ = 7U;
constexpr unsigned int A10C_VAMFREQ_ADDR = 0x12de;
constexpr unsigned int A10C_VAMMODE_ADDR = 0x1186;
constexpr unsigned int A10C_VAMMODE_MASK = 0xc00;
constexpr unsigned int A10C_VAMMODE_SHIFT = 10U;
constexpr uint8_t A10C_VAMPSETSW_SZ = 2U;
constexpr unsigned int A10C_VAMPSETSW_ADDR = 0x118a;
constexpr uint8_t A10C_VFMFREQ_SZ = 7U;
constexpr unsigned int A10C_VFMFREQ_ADDR = 0x12e6;
constexpr unsigned int A10C_VFMMODE_ADDR = 0x1194;
constexpr unsigned int A10C_VFMMODE_MASK = 0x0180;
constexpr unsigned int A10C_VFMMODE_SHIFT = 7U;
constexpr uint8_t A10C_VFMPSETSW_SZ = 2U;
constexpr unsigned int A10C_VFMPSETSW_ADDR = 0x1196;
constexpr uint8_t A10C_TCNCHNL_SZ = 4U;
constexpr unsigned int A10C_TCNCHNL_ADDR = 0x1162;
constexpr unsigned int A10C_TCNMODE_ADDR = 0x1168;
constexpr unsigned int A10C_TCNMODE_MASK = 0x000e;
constexpr unsigned int A10C_TCNMODE_SHIFT = 1U;
constexpr uint8_t A10C_ILSFREQ_SZ = 6U;
constexpr unsigned int A10C_ILSFREQ_ADDR = 0x12d8;
constexpr unsigned int A10C_HSIHDG_ADDR = 0x104c;
constexpr unsigned int A10C_HSIHDG_MASK = 0xffff;
constexpr unsigned char A10C_HSIHDG_SHIFT = 0U;
constexpr unsigned int A10C_HSIHDGBUG_ADDR = 0x1052;
constexpr unsigned int A10C_HSIHDGBUG_MASK = 0xffff;
constexpr unsigned char A10C_HSIHDGBUG_SHIFT = 0U;
constexpr unsigned int A10C_HSICRS_ADDR = 0x1054;
constexpr unsigned int A10C_HSICRS_MASK = 0xffff;
constexpr unsigned char A10C_HSICRS_SHIFT = 0U;
constexpr unsigned int A10C_MASTERCAUTLT_ADDR = 0x1012;
constexpr unsigned int A10C_MASTERCAUTLT_MASK = 0x0800;
constexpr unsigned char A10C_MASTERCAUTLT_SHIFT = 11U;
constexpr unsigned int A10C_MASTERARMSW_ADDR = 0x10e8;
constexpr unsigned int A10C_MASTERARMSW_MASK = 0x000c;
constexpr unsigned char A10C_MASTERARMSW_SHIFT = 2U;
constexpr unsigned int A10C_GUNRDYLT_ADDR = 0x1026;
constexpr unsigned int A10C_GUNRDYLT_MASK = 0x8000;
constexpr unsigned char A10C_GUNRDYLT_SHIFT = 15U;

// F-16C
constexpr unsigned int F16C_DED_SZ = 25U;
constexpr unsigned int F16C_DEDLINE1_ADDR = 0x44fc;
constexpr unsigned int F16C_DEDLINE2_ADDR = 0x4516;
constexpr unsigned int F16C_DEDLINE3_ADDR = 0x4530;
constexpr unsigned int F16C_DEDLINE4_ADDR = 0x454a;
constexpr unsigned int F16C_DEDLINE5_ADDR = 0x4564;
constexpr unsigned int F16C_FUELQTYKNOB_ADDR = 0x441e;
constexpr unsigned int F16C_FUELQTYKNOB_MASK = 0x0007;
constexpr unsigned char F16C_FUELQTYKNOB_SHIFT = 0U;
constexpr unsigned int F16C_FUELQTYAL_ADDR = 0x44dc;
constexpr unsigned int F16C_FUELQTYAL_MASK = 0xffff;
constexpr unsigned char F16C_FUELQTYAL_SHIFT = 0U;
constexpr unsigned int F16C_FUELQTYFR_ADDR = 0x44de;
constexpr unsigned int F16C_FUELQTYFR_MASK = 0xffff;
constexpr unsigned char F16C_FUELQTYFR_SHIFT = 0U;
constexpr unsigned int F16C_FUELTOT100_ADDR = 0x44e4;
constexpr unsigned int F16C_FUELTOT100_MASK = 0xffff;
constexpr unsigned char F16C_FUELTOT100_SHIFT = 0U;
constexpr unsigned int F16C_FUELTOT1K_ADDR = 0x44e2;
constexpr unsigned int F16C_FUELTOT1K_MASK = 0xffff;
constexpr unsigned char F16C_FUELTOT1K_SHIFT = 0U;
constexpr unsigned int F16C_FUELTOT10K_ADDR = 0x44e0;
constexpr unsigned int F16C_FUELTOT10K_MASK = 0xffff;
constexpr unsigned char F16C_FUELTOT10K_SHIFT = 0U;
constexpr unsigned int F16C_MASTERCAUTLT_ADDR = 0x4472;
constexpr unsigned int F16C_MASTERCAUTLT_MASK = 0x0800;
constexpr unsigned char F16C_MASTERCAUTLT_SHIFT = 11U;
constexpr unsigned int F16C_MASTERARMSW_ADDR = 0x4424;
constexpr unsigned int F16C_MASTERARMSW_MASK = 0x0030;
constexpr unsigned char F16C_MASTERARMSW_SHIFT = 4U;
constexpr unsigned int F16C_STORESCFGSW_ADDR = 0x4400;
constexpr unsigned int F16C_STORESCFGSW_MASK = 0x0080;
constexpr unsigned char F16C_STORESCFGSW_SHIFT = 7U;

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
constexpr unsigned int FA18C_MASTERARMSW_ADDR = 0x740c;
constexpr unsigned int FA18C_MASTERARMSW_MASK = 0x2000;
constexpr unsigned char FA18C_MASTERARMSW_SHIFT = 13U;
constexpr unsigned int FA18C_LTDRSW_ADDR = 0x74c4;
constexpr unsigned int FA18C_LTDRSW_MASK = 0x0100;
constexpr unsigned char FA18C_LTDRSW_SHIFT = 8U;

// M2000C

constexpr unsigned int M2000C_PCNLEFT_ADDR = 0x72ec;
constexpr uint8_t M2000C_PCNLEFT_SZ = 8U;
constexpr unsigned int M2000C_PCNRIGHT_ADDR = 0x72f6;
constexpr uint8_t M2000C_PCNRIGHT_SZ = 9U;
constexpr unsigned int M2000C_PCNDLEFT_ADDR = 0x7304;
constexpr uint8_t M2000C_PCNDLEFT_SZ = 2U;
constexpr unsigned int M2000C_PCNDRIGHT_ADDR = 0x7306;
constexpr uint8_t M2000C_PCNDRIGHT_SZ = 2U;
constexpr unsigned int M2000C_PCNDISPPREP_ADDR = 0x72f4;
constexpr uint8_t M2000C_PCNDISPPREP_SZ = 2U;
constexpr unsigned int M2000C_PCNDISPDEST_ADDR = 0x72ea;
constexpr uint8_t M2000C_PCNDISPDEST_SZ = 2U;
constexpr unsigned int M2000C_PCNMODEROT_ADDR = 0x72ea;
constexpr unsigned int M2000C_PCNSELROT_ADDR = 0x72c4;
constexpr unsigned int M2000C_PCNSELROT_MASK = 0xf000;
constexpr unsigned int M2000C_PCNSELROT_SHIFT = 12U;
constexpr unsigned int M2000C_PCNBTNLT_ADDR = 0x72d4;
constexpr unsigned int M2000C_PCNPREPBTNLT_MASK = 0x0100;
constexpr unsigned int M2000C_PCNDESTBTNLT_MASK = 0x0200;
constexpr unsigned int M2000C_PCNBADBTNLT_MASK = 0x0400;
constexpr unsigned int M2000C_PCNRECBTNLT_MASK = 0x0800;
constexpr unsigned int M2000C_PCNEFFBTNLT_MASK = 0x1000;
constexpr unsigned int M2000C_PCNINSBTNLT_MASK = 0x2000;
constexpr unsigned int M2000C_PCNVALBTNLT_MASK = 0x4000;
constexpr unsigned int M2000C_PCNMRCBTNLT_MASK = 0x8000;
constexpr unsigned char M2000C_PCNPREPBTNLT_SHIFT = 8U;
constexpr unsigned char M2000C_PCNDESTBTNLT_SHIFT = 9U;
constexpr unsigned char M2000C_PCNBADBTNLT_SHIFT = 10U;
constexpr unsigned char M2000C_PCNRECBTNLT_SHIFT = 11U;
constexpr unsigned char M2000C_PCNEFFBTNLT_SHIFT = 12U;
constexpr unsigned char M2000C_PCNINSBTNLT_SHIFT = 13U;
constexpr unsigned char M2000C_PCNVALBTNLT_SHIFT = 14U;
constexpr unsigned char M2000C_PCNMRCBTNLT_SHIFT = 15U;
constexpr unsigned int M2000C_PCNPANLT_ADDR = 0x72d0;
constexpr unsigned int M2000C_PCNPRETPANLT_MASK = 0x0400;
constexpr unsigned int M2000C_PCNALNPANLT_MASK = 0x0800;
constexpr unsigned int M2000C_PCNMIPPANLT_MASK = 0x1000;
constexpr unsigned int M2000C_PCNNDEGPANLT_MASK = 0x2000;
constexpr unsigned int M2000C_PCNSECPANLT_MASK = 0x4000;
constexpr unsigned int M2000C_PCNUNIPANLT_MASK = 0x8000;
constexpr unsigned char M2000C_PCNPRETPANLT_SHIFT = 10U;
constexpr unsigned char M2000C_PCNALNPANLT_SHIFT = 11U;
constexpr unsigned char M2000C_PCNMIPPANLT_SHIFT = 12U;
constexpr unsigned char M2000C_PCNNDEGPANLT_SHIFT = 13U;
constexpr unsigned char M2000C_PCNSECPANLT_SHIFT = 14U;
constexpr unsigned char M2000C_PCNUNIPANLT_SHIFT = 15U;
constexpr unsigned int M2000C_PCNMEMLT_ADDR = 0x72d8;
constexpr unsigned int M2000C_PCNM91MEMLT_MASK = 0x0200;
constexpr unsigned int M2000C_PCNM92MEMLT_MASK = 0x0400;
constexpr unsigned int M2000C_PCNM93MEMLT_MASK = 0x0800;
constexpr unsigned char M2000C_PCNM91MEMLT_SHIFT = 9U;
constexpr unsigned char M2000C_PCNM92MEMLT_SHIFT = 10U;
constexpr unsigned char M2000C_PCNM93MEMLT_SHIFT = 11U;
constexpr unsigned int M2000C_PANNELT_ADDR = 0x72aa;
constexpr unsigned char M2000C_PANNELT_SHIFT = 11U;
constexpr unsigned int M2000C_PANNEAMBERLT_MASK = 0x0800;
constexpr unsigned int M2000C_PANNEREDLT_MASK = 0x1000;


/*************/
/* Variables */
/*************/

static Mode WorkMode;

// Function to call for special procesing of input before sending DX events
// It depends on the WorkMode and returns true when standard processing should
// follow.
static bool (*pModeProcessEv)(const Event &Ev, const Directx::Event_t &EvDx) =
  nullptr;

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
 *   Callback to update A-10C UHF radio frequency.
 */
static void cbA10cUhfFreq(char *szValue)
{
  DiPnl.a10cUhfFreq(szValue);
}

/*
 *   Callback to update A-10C UHF radio mode.
 */
static void cbA10cUhfMode(unsigned int Value)
{
  DiPnl.a10cUhfMode(Value);
}

/*
 *   Callback to update A-10C UHF radio preset channel.
 */
static void cbA10cUhfPset(char *szValue)
{
  DiPnl.a10cUhfPreset(szValue);
}

/*
 *   Callback to update A-10C VHF AM radio frequency.
 */
static void cbA10cVamFreq(char *szValue)
{
  DiPnl.a10cVamFreq(szValue);
}

/*
 *   Callback to update A-10C VHF AM radio mode.
 */
static void cbA10cVamMode(unsigned int Value)
{
  DiPnl.a10cVamMode(Value);
}

/*
 *   Callback to update A-10C VHF AM radio preset channel.
 */
static void cbA10cVamPset(char *szValue)
{
  DiPnl.a10cVamPreset(szValue);
}


/*
 *   Callback to update A-10C VHF FM radio frequency.
 */
static void cbA10cVfmFreq(char *szValue)
{
  DiPnl.a10cVfmFreq(szValue);
}

/*
 *   Callback to update A-10C VHF FM radio mode.
 */
static void cbA10cVfmMode(unsigned int Value)
{
  DiPnl.a10cVfmMode(Value);
}

/*
 *   Callback to update A-10C VHF FM radio preset channel.
 */
static void cbA10cVfmPset(char *szValue)
{
  DiPnl.a10cVfmPreset(szValue);
}

/*
 *   Callback to update A-10C TACAN radio channel.
 */
static void cbA10cTcnChannel(char *szValue)
{
  DiPnl.a10cTcnChannel(szValue);
}

/*
 *   Callback to update A-10C TACAN radio mode.
 */
static void cbA10cTcnMode(unsigned int Value)
{
  DiPnl.a10cTcnMode(Value);
}

/*
 *   Callback to update A-10C ILS radio frequency.
 */
static void cbA10cIlsFreq(char *szValue)
{
  DiPnl.a10cIlsFreq(szValue);
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

  // Register special processing function
  pModeProcessEv = a10cProcessEv;

  // Register callbacks creating DCS-BIOS handlers in heap memory

  // CDU Scratchpad
  new DcsBios::StringBuffer<A10C_CDUSP_SZ>(A10C_CDUSP_ADDR, cbA10cCduScrpad);

  // Radios
  new DcsBios::StringBuffer<A10C_UHFFREQ_SZ>(
      A10C_UHFFREQ_ADDR, cbA10cUhfFreq);
  new DcsBios::IntegerBuffer(A10C_UHFMODE_ADDR, A10C_UHFMODE_MASK,
      A10C_UHFMODE_SHIFT, cbA10cUhfMode);
  new DcsBios::StringBuffer<A10C_UHFPSET_SZ>(
      A10C_UHFPSET_ADDR, cbA10cUhfPset);
  new DcsBios::StringBuffer<A10C_VAMFREQ_SZ>(
      A10C_VAMFREQ_ADDR, cbA10cVamFreq);
  new DcsBios::IntegerBuffer(A10C_VAMMODE_ADDR, A10C_VAMMODE_MASK,
      A10C_VAMMODE_SHIFT, cbA10cVamMode);
  new DcsBios::StringBuffer<A10C_VAMPSETSW_SZ>(
      A10C_VAMPSETSW_ADDR, cbA10cVamPset);
  new DcsBios::StringBuffer<A10C_VFMFREQ_SZ>(
      A10C_VFMFREQ_ADDR, cbA10cVfmFreq);
  new DcsBios::IntegerBuffer(A10C_VFMMODE_ADDR, A10C_VFMMODE_MASK,
      A10C_VFMMODE_SHIFT, cbA10cVfmMode);
  new DcsBios::StringBuffer<A10C_VFMPSETSW_SZ>(
      A10C_VFMPSETSW_ADDR, cbA10cVfmPset);
  new DcsBios::StringBuffer<A10C_TCNCHNL_SZ>(
      A10C_TCNCHNL_ADDR, cbA10cTcnChannel);
  new DcsBios::IntegerBuffer(A10C_TCNMODE_ADDR, A10C_TCNMODE_MASK,
      A10C_TCNMODE_SHIFT, cbA10cTcnMode);
  new DcsBios::StringBuffer<A10C_ILSFREQ_SZ>(
      A10C_ILSFREQ_ADDR, cbA10cIlsFreq);

  // Heading and course
  new DcsBios::IntegerBuffer(A10C_HSIHDG_ADDR, A10C_HSIHDG_MASK,
      A10C_HSIHDG_SHIFT, cbA10cHsiHdg);
  new DcsBios::IntegerBuffer(A10C_HSIHDGBUG_ADDR, A10C_HSIHDGBUG_MASK,
      A10C_HSIHDGBUG_SHIFT, cbA10cHsiHdgBug);
  new DcsBios::IntegerBuffer(A10C_HSICRS_ADDR, A10C_HSICRS_MASK,
      A10C_HSICRS_SHIFT, cbA10cHsiCrs);

  // LED lights
  new DcsBios::IntegerBuffer(A10C_MASTERCAUTLT_ADDR, A10C_MASTERCAUTLT_MASK,
      A10C_MASTERCAUTLT_SHIFT, cbA10cMasterCautLt);
  new DcsBios::IntegerBuffer(A10C_MASTERARMSW_ADDR, A10C_MASTERARMSW_MASK,
      A10C_MASTERARMSW_SHIFT, cbA10cMasterArmSw);
  new DcsBios::IntegerBuffer(A10C_GUNRDYLT_ADDR, A10C_GUNRDYLT_MASK,
      A10C_GUNRDYLT_SHIFT, cbA10cGunReadyLt);
}


/* DCS-BIOS callbacks for F-16C */

/*
 *   Callback to update F-16C DED Line 1.
 */
static void cbF16cDedLine1(char *szValue)
{
  DiPnl.f16cDed(0U, szValue);
}

/*
 *   Callback to update F-16C DED Line 2.
 */
static void cbF16cDedLine2(char *szValue)
{
  DiPnl.f16cDed(1U, szValue);
}

/*
 *   Callback to update F-16C DED Line 3.
 */
static void cbF16cDedLine3(char *szValue)
{
  DiPnl.f16cDed(2U, szValue);
}

/*
 *   Callback to update F-16C DED Line 4.
 */
static void cbF16cDedLine4(char *szValue)
{
  DiPnl.f16cDed(3U, szValue);
}

/*
 *   Callback to update F-16C DED Line 5.
 */
static void cbF16cDedLine5(char *szValue)
{
  DiPnl.f16cDed(4U, szValue);
}

/*
 *   Callback to update F-16C Fuel quantity selecion knob position.
 */
static void cbF16cFuelQtySelKnob(unsigned int Value)
{
  DiPnl.f16cFuelQtySelKnob((uint8_t) Value);
}

/*
 *   Callback to update F-16C Fuel quantity AL indicator.
 */
static void cbF16cFuelQtyIndAl(unsigned int Value)
{
  DiPnl.f16cFuelQtyIndicator(false, Value);
}

/*
 *   Callback to update F-16C Fuel quantity FR indicator.
 */
static void cbF16cFuelQtyIndFr(unsigned int Value)
{
  DiPnl.f16cFuelQtyIndicator(true, Value);
}

/*
 *   Callback to update F-16C hundreds of the fuel totalizer counter.
 */
static void cbF16cFuelTotCounter100(unsigned int Value)
{
  DiPnl.f16cFuelTotalizerHundreds(Value);
}

/*
 *   Callback to update F-16C thousands of the fuel totalizer counter.
 */
static void cbF16cFuelTotCounter1k(unsigned int Value)
{
  DiPnl.f16cFuelTotalizerThousands(false, Value);
}

/*
 *   Callback to update F-16C tens of thousands of the fuel totalizer counter.
 */
static void cbF16cFuelTotCounter10k(unsigned int Value)
{
  DiPnl.f16cFuelTotalizerThousands(true, Value);
}

/*
 *   Callback to update F-16C Master Caution light.
 */
static void cbF16cMasterCautLt(unsigned int Value)
{
  DiPnl.f16cMasterCaut((uint8_t) Value);
}

/*
 *   Callback to update F-16C Master Arm switch position.
 */
static void cbF16cMasterArmSw(unsigned int Value)
{
  DiPnl.f16cMasterArm((uint8_t) Value);
}

/*
 *   Callback to update F-16C stores configuration category switch position.
 */
static void cbF16cStoresCat(unsigned int Value)
{
  DiPnl.f16cStoresCat((uint8_t) Value);
}

/*
 *   Initializes F-16C mode.
 */
static void modeF16cInit()
{
  // Initializes display
  DiPnl.f16cStart();

  // Register special processing function
  pModeProcessEv = f16cProcessEv;

  // Tune encoder DX event delays for HDG/CRS and ICP Up/Dn
  SwPnl.setEncDelay(SwitchPnl::EncLeft, F16C_ICP_DX_DELAY_PR, DX_DELAY_RP);

  // Register callbacks creating DCS-BIOS handlers in heap memory

  // DED
  new DcsBios::StringBuffer<F16C_DED_SZ>(F16C_DEDLINE1_ADDR, cbF16cDedLine1);
  new DcsBios::StringBuffer<F16C_DED_SZ>(F16C_DEDLINE2_ADDR, cbF16cDedLine2);
  new DcsBios::StringBuffer<F16C_DED_SZ>(F16C_DEDLINE3_ADDR, cbF16cDedLine3);
  new DcsBios::StringBuffer<F16C_DED_SZ>(F16C_DEDLINE4_ADDR, cbF16cDedLine4);
  new DcsBios::StringBuffer<F16C_DED_SZ>(F16C_DEDLINE5_ADDR, cbF16cDedLine5);

  // Fuel
  new DcsBios::IntegerBuffer(F16C_FUELQTYKNOB_ADDR, F16C_FUELQTYKNOB_MASK,
      F16C_FUELQTYKNOB_SHIFT, cbF16cFuelQtySelKnob);
  new DcsBios::IntegerBuffer(F16C_FUELQTYAL_ADDR, F16C_FUELQTYAL_MASK,
      F16C_FUELQTYAL_SHIFT, cbF16cFuelQtyIndAl);
  new DcsBios::IntegerBuffer(F16C_FUELQTYFR_ADDR, F16C_FUELQTYFR_MASK,
      F16C_FUELQTYFR_SHIFT, cbF16cFuelQtyIndFr);
  new DcsBios::IntegerBuffer(F16C_FUELTOT100_ADDR, F16C_FUELTOT100_MASK,
      F16C_FUELTOT100_SHIFT, cbF16cFuelTotCounter100);
  new DcsBios::IntegerBuffer(F16C_FUELTOT1K_ADDR, F16C_FUELTOT1K_MASK,
      F16C_FUELTOT1K_SHIFT, cbF16cFuelTotCounter1k);
  new DcsBios::IntegerBuffer(F16C_FUELTOT10K_ADDR, F16C_FUELTOT10K_MASK,
      F16C_FUELTOT10K_SHIFT, cbF16cFuelTotCounter10k);

  // LED lights
  new DcsBios::IntegerBuffer(F16C_MASTERCAUTLT_ADDR, F16C_MASTERCAUTLT_MASK,
      F16C_MASTERCAUTLT_SHIFT, cbF16cMasterCautLt);
  new DcsBios::IntegerBuffer(F16C_MASTERARMSW_ADDR, F16C_MASTERARMSW_MASK,
      F16C_MASTERARMSW_SHIFT, cbF16cMasterArmSw);
  new DcsBios::IntegerBuffer(F16C_STORESCFGSW_ADDR, F16C_STORESCFGSW_MASK,
      F16C_STORESCFGSW_SHIFT, cbF16cStoresCat);
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

  // Master Arm and LTD/R switches
  new DcsBios::IntegerBuffer(FA18C_MASTERARMSW_ADDR, FA18C_MASTERARMSW_MASK,
      FA18C_MASTERARMSW_SHIFT, cbFa18cMasterArmSw);
  new DcsBios::IntegerBuffer(FA18C_LTDRSW_ADDR, FA18C_LTDRSW_MASK,
      FA18C_LTDRSW_SHIFT, cbFa18cLtdrSw);
}


/*
 *   Callback to update M2000C PCN left digit display.
 */
static void m2000cPcnDigLeft(char *szValue)
{
  DiPnl.m2000cPcnDigLeft(szValue);
}

/*
 *   Callback to update M2000C PCN right digit display.
 */
static void m2000cPcnDigRight(char *szValue)
{
  DiPnl.m2000cPcnDigRight(szValue);
}

/*
 *   Callback to update M2000C PCN left display.
 */
static void m2000cPcnLeft(char *szValue)
{
  DiPnl.m2000cPcnLeft(szValue);
}

/*
 *   Callback to update M2000C PCN right display.
 */
static void m2000cPcnRight(char *szValue)
{
  DiPnl.m2000cPcnRight(szValue);
}

/*
 *   Callback to update M2000C PCN Prep display.
 */
static void m2000cPcnDispPrep(char *szValue)
{
  DiPnl.m2000cPcnPrep(szValue);
}

/*
 *   Callback to update M2000C PCN Dest display.
 */
static void m2000cPcnDispDest(char *szValue)
{
  DiPnl.m2000cPcnDest(szValue);
}

/*
 *   Callback on change of M2000C PCN selector mode rotary switch.
 */
static void cbM2000cPcnSelectorRot(unsigned int Value)
{
  DiPnl.m2000cPcnMode((uint8_t) Value);
}

/*
 *   Callback on change of M2000C UNC button lights.
 */
static void cbM2000cPcnBtnLt(unsigned int Value)
{
  DiPnl.m2000cPcnButtonLt(highByte(Value));
}

/*
 *   Callback on change of M2000C UNC panne lights.
 */
static void cbM2000cPcnPanLt(unsigned int Value)
{
  DiPnl.m2000cPcnPanneLt(highByte(Value));
}

/*
 *   Callback on change of M2000C UNC M lights.
 */
static void cbM2000cPcnMemLt(unsigned int Value)
{
  DiPnl.m2000cPcnMemLt(highByte(Value));
}

/*
 *   Callback on change of M2000C Panne lights.
 */
static void cbM2000cPanneLt(unsigned int Value)
{
  // Send both Amber (bit 0) and Red (bit 1) Panne lights status
  DiPnl.m2000cPanneLt((uint8_t) Value);
}

/*
 *   Initializes M2000C mode.
 */
static void modeM2000cInit()
{
  // Initializes display
  DiPnl.m2000cStart();

  // Register callbacks creating DCS-BIOS handlers in heap memory
  // PCN
  new DcsBios::StringBuffer<M2000C_PCNDLEFT_SZ>(
      M2000C_PCNDLEFT_ADDR, m2000cPcnDigLeft);
  new DcsBios::StringBuffer<M2000C_PCNLEFT_SZ>(
      M2000C_PCNLEFT_ADDR, m2000cPcnLeft);
  new DcsBios::StringBuffer<M2000C_PCNDRIGHT_SZ>(
      M2000C_PCNDRIGHT_ADDR, m2000cPcnDigRight);
  new DcsBios::StringBuffer<M2000C_PCNRIGHT_SZ>(
      M2000C_PCNRIGHT_ADDR, m2000cPcnRight);
  new DcsBios::StringBuffer<M2000C_PCNDISPPREP_SZ>(
      M2000C_PCNDISPPREP_ADDR, m2000cPcnDispPrep);
  new DcsBios::StringBuffer<M2000C_PCNDISPDEST_SZ>(
      M2000C_PCNDISPDEST_ADDR, m2000cPcnDispDest);

  // PCN Selector Mode rotary
  new DcsBios::IntegerBuffer(M2000C_PCNSELROT_ADDR, M2000C_PCNSELROT_MASK,
      M2000C_PCNSELROT_SHIFT, cbM2000cPcnSelectorRot);

  // PCN lights
  new DcsBios::IntegerBuffer(M2000C_PCNBTNLT_ADDR,
      M2000C_PCNPREPBTNLT_MASK | M2000C_PCNDESTBTNLT_MASK |
      M2000C_PCNBADBTNLT_MASK | M2000C_PCNRECBTNLT_MASK |
      M2000C_PCNEFFBTNLT_MASK | M2000C_PCNINSBTNLT_MASK |
      M2000C_PCNVALBTNLT_MASK | M2000C_PCNMRCBTNLT_MASK,
      (unsigned char) 0U, cbM2000cPcnBtnLt);
  new DcsBios::IntegerBuffer(M2000C_PCNPANLT_ADDR,
      M2000C_PCNPRETPANLT_MASK | M2000C_PCNALNPANLT_MASK |
      M2000C_PCNMIPPANLT_MASK | M2000C_PCNNDEGPANLT_MASK |
      M2000C_PCNSECPANLT_MASK | M2000C_PCNUNIPANLT_MASK,
      (unsigned char) 0U, cbM2000cPcnPanLt);
  new DcsBios::IntegerBuffer(M2000C_PCNMEMLT_ADDR,
      M2000C_PCNM91MEMLT_MASK | M2000C_PCNM92MEMLT_MASK |
      M2000C_PCNM93MEMLT_MASK,
      (unsigned char) 0U, cbM2000cPcnMemLt);

  // Caution lights: both amber and red on same callback
  new DcsBios::IntegerBuffer(M2000C_PANNELT_ADDR,
      M2000C_PANNEAMBERLT_MASK | M2000C_PANNEREDLT_MASK,
      M2000C_PANNELT_SHIFT, cbM2000cPanneLt);
}


/*
 *   Initializes debug mode
 */
static void modeDebugInit()
{
  // Init display panel
  DiPnl.debugStart();

  // Prepare function to display event information
  pModeProcessEv = debugProcessEv;
}




/*
 *   A-10C mode pModeProcessEv function. Performs special processing of the
 *  input for buggy DCS controls such as HDG & CRS knobs that do not work
 *  correctly with DX events.
 *   Parameters:
 *   * Ev: Event that was registered, cannot be EvNone.
 *   * EvDx: DirectX event to be sent (not used in this function)
 *  Returns:
 *  * true: when the Event was processed and a corresponding action was issued
 *  * false: nothing done, standard DX event should be issued 
 */
#pragma GCC diagnostic push
// Disable: warning: unused parameter '' [-Wunused-parameter]
#pragma GCC diagnostic ignored "-Wunused-parameter"

static bool a10cProcessEv(const Event &Ev, const Directx::Event_t &EvDx)
{
  constexpr uint8_t MSG_POS = 4U;
  constexpr uint8_t ARG_POS = 0U;
  static char Msg[] = "HSI_xxx_KNOB";
  static char Arg[] = "x1144";
  bool Processed = false;
  bool Send = false;

  // Only interested on encoder events
  if (Ev.isEnc())
  {

#pragma GCC diagnostic push
// Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

    // Only interested on encoders EncCom1 & EncCom2
    switch (Ev.EncId)
    {
    case SwitchPnl::EncCom1:  // HDG
      switch (Ev.Id)
      {
      case Event::EvEncCwPress:
        Arg[ARG_POS] = _BIOS_ARG_INC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_HDG, sizeof _BIOS_MSG_HDG);
        Send = true;
        break;
      case Event::EvEncCcwPress:
        Arg[ARG_POS] = _BIOS_ARG_DEC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_HDG, sizeof _BIOS_MSG_HDG);
        Send = true;
        break;
      }
      Processed = true;
      break;
    case SwitchPnl::EncCom2:  // CRS
      switch (Ev.Id)
      {
      case Event::EvEncCwPress:
        Arg[ARG_POS] = _BIOS_ARG_INC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_CRS, sizeof _BIOS_MSG_CRS);
        Send = true;
        break;
      case Event::EvEncCcwPress:
        Arg[ARG_POS] = _BIOS_ARG_DEC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_CRS, sizeof _BIOS_MSG_CRS);
        Send = true;
        break;
      }
      Processed = true;
      break;
    }

#pragma GCC diagnostic pop

    // Send the message
    if (Send)
      DcsBios::sendDcsBiosMessage(Msg, Arg);
  }

  return Processed;
}

#pragma GCC diagnostic pop


/*
 *   F-16C mode pModeProcessEv function. Performs special processing of the
 *  input for buggy DCS controls such as HDG & CRS knobs that do not work
 *  correctly with DX events (they need very large push times).
 *   Parameters:
 *   * Ev: Event that was registered, cannot be EvNone.
 *   * EvDx: DirectX event to be sent (not used in this function)
 *  Returns:
 *  * true: when the Event was processed and a corresponding action was issued
 *  * false: nothing done, standard DX event should be issued 
 */
#pragma GCC diagnostic push
// Disable: warning: unused parameter '' [-Wunused-parameter]
#pragma GCC diagnostic ignored "-Wunused-parameter"

static bool f16cProcessEv(const Event &Ev, const Directx::Event_t &EvDx)
{
  constexpr uint8_t MSG_POS = 5U;
  constexpr uint8_t ARG_POS = 0U;
  static char Msg[] = "EHSI_xxx_SET_KNB";
  static char Arg[] = "x9";
  bool Processed = false;
  bool Send = false;

  // Only interested on encoder events
  if (Ev.isEnc())
  {

#pragma GCC diagnostic push
// Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

    // Only interested on encoders EncCom1 & EncCom2
    switch (Ev.EncId)
    {
    case SwitchPnl::EncCom1:  // HDG
      switch (Ev.Id)
      {
      case Event::EvEncCwPress:
        Arg[ARG_POS] = _BIOS_ARG_INC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_HDG, sizeof _BIOS_MSG_HDG);
        Send = true;
        break;
      case Event::EvEncCcwPress:
        Arg[ARG_POS] = _BIOS_ARG_DEC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_HDG, sizeof _BIOS_MSG_HDG);
        Send = true;
        break;
      }
      Processed = true;
      break;
    case SwitchPnl::EncCom2:  // CRS
      switch (Ev.Id)
      {
      case Event::EvEncCwPress:
        Arg[ARG_POS] = _BIOS_ARG_INC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_CRS, sizeof _BIOS_MSG_CRS);
        Send = true;
        break;
      case Event::EvEncCcwPress:
        Arg[ARG_POS] =_BIOS_ARG_DEC_CHAR;
        memcpy_P(Msg + MSG_POS, _BIOS_MSG_CRS, sizeof _BIOS_MSG_CRS);
        Send = true;
        break;
      }
      Processed = true;
      break;
    }

#pragma GCC diagnostic pop

    // Send the message
    if (Send)
      DcsBios::sendDcsBiosMessage(Msg, Arg);
  }

  return Processed;
}

#pragma GCC diagnostic pop


/*
 *   DEBUG mode pModeProcessEv function. Performs special processing of the
 *  input displaying the event on the LCD.
 *   Parameters:
 *   * Ev: Event that was registered, cannot be EvNone.
 *   * EvDx: DirectX event to be sent
 *  Returns:
 *  * false: standard DX event should be issued 
 */
static bool debugProcessEv(const Event &Ev, const Directx::Event_t &EvDx)
{
  DiPnl.debugShowEvent(Ev, EvDx);

  return false;
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

    // Do special processing first if configured so
    if (pModeProcessEv==nullptr || !(*pModeProcessEv)(Ev, EvDx))
    {
      // Continue with std processing when special not configured or says so

      // Send DirectX event to PC
      if (EvDx.Action == Directx::AcRelease)
        Joy.releaseButton(EvDx.Button);
      else
        Joy.pressButton(EvDx.Button);
    }
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
  case Mode::M_F16C:
    modeF16cInit();
    break;
  case Mode::M_FA18C:
    modeFa18cInit();
    break;
  case Mode::M_M2000C:
    modeM2000cInit();
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
