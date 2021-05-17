#include "directx.h"


/********************/
/* Static constants */
/********************/

// KP0: converts keypad matrix button id to DX: [0, 15]
const uint8_t Directx::_KP0_BTN_DX[KP0_NUM_KEYS] =
{
  12,  // A/P
  13,  // TCN
  14,  // ILS
  15,  // ON
   2,  // 3
   5,  // 6
   8,  // 9
  11,  // ENT
   1,  // 2
   4,  // 5
   7,  // 8
   9,  // 0
   0,  // 1
   3,  // 4
   6,  // 7
  10   // CLR
};

// KP1: converts keypad matrix button id to DX: [16, 23], 28, 29, 38
const uint8_t Directx::_KP1_BTN_DX[KP1_NUM_KEYS] =
{
  20,  // Left LSK1
  19,  // CRS Right
  28,  // COM1
  21,  // Left LSK2
  18,  // CRS Left
  17,  // HDG Right
  22,  // Left LSK3
  29,  // COM2
  16,  // HDG Left
  23,  // Left LSK4
  38  // Master Warning
};

// KP2: converts keypad matrix button id to DX: [24, 27], [30, 37]
const uint8_t Directx::_KP2_BTN_DX[KP2_NUM_KEYS] =
{
  24,  // Right LSK1
  30,  // Encoder left (AMPCD OBS RNG)
  31,  // Encoder right (AMPCD OBS SEQ)
  25,  // Right LSK2
  32,  // Button (AMPCD OBS TCN)
  33,  // Button (AMPCD OBS AUTO)
  26,  // Right LSK3
  34,  // Button (AMPCD OBS TCN)
  35,  // Button (AMPCD OBS WPT)
  27,  // Right LSK4
  36,  // Button (AMPCD OBS MK)
  37   // Button (AMPCD OBS WPDSG)
};

// Label to access all KP*_BTN_DX through indexes
const uint8_t *const Directx::_KP_BTN_DX[NUM_KP] =
{
  _KP0_BTN_DX,
  _KP1_BTN_DX,
  _KP2_BTN_DX
};


/******************/
/* Member methods */
/******************/

/*
 *   Translates a keypad key or rotary encoder event into a DirectX button
 *  event.  Note that an Ev EvNone will return an undefined Dx event.
 *   Parameters:
 *   * Ev: the keypad/encoder event to translate.
 *   Returns: the DirectX translated event.
 */
Directx::Event_t Directx::translate(const Event &Ev)
{
  Event_t EvDx;

#pragma GCC diagnostic push
  // Disable: warning: enumeration value 'EvNone' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

  switch (Ev.Id)
  {
  case Event::EvKpPress:
    EvDx.Button = _KP_BTN_DX[Ev.Kp.KpId][Ev.Kp.KeyId];
    EvDx.Action = AcPress;
    break;
  case Event::EvKpRelease:
    EvDx.Button = _KP_BTN_DX[Ev.Kp.KpId][Ev.Kp.KeyId];
    EvDx.Action = AcRelease;
    break;
  case Event::EvEncCcwPress:
    EvDx.Button = _encDx(Ev.EncId, _EncDirACcw);
    EvDx.Action = AcPress;
    break;
  case Event::EvEncCcwRelease:
    EvDx.Button = _encDx(Ev.EncId, _EncDirACcw);
    EvDx.Action = AcRelease;
    break;
  case Event::EvEncCwPress:
    EvDx.Button = _encDx(Ev.EncId, _EncDirACw);
    EvDx.Action = AcPress;
    break;
  case Event::EvEncCwRelease:
    EvDx.Button = _encDx(Ev.EncId, _EncDirACw);
    EvDx.Action = AcRelease;
    break;
  }

#pragma GCC diagnostic pop

  return EvDx;
}


/*
 *   Calculates DirectX button from a rotary encoder and its direction of
 *  rotation.
 *   Parameters:
 *   * EncId: encoder identifier
 *   * EncDir: encoder direction of rotation
 */
inline uint8_t Directx::_encDx(uint8_t EncId, _EncDirArray EncDir)
{
  return _ENC_DX_BASE + (EncId * ENC_NUM_PINS) + EncDir;
}
