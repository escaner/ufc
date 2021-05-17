#ifndef _DX_H_
#define _DX_H_


#include "config.h"
#include "event.h"


/*
 *   Class to translate keypad and rotary encoder events into DirectX
 *  buttons.
 */
class Directx
{
public:
  /**************/
  /* Data types */
  /**************/

  enum Action_t: int8_t { AcRelease, AcPress };

  struct Event_t
  {
    uint8_t Button;
    Action_t Action;
  };

  /******************/
  /* Public methods */
  /******************/

  static Event_t translate(const Event &KeyEv);

protected:

  /********************/
  /* Static constants */
  /********************/

  // Converts keypad matrix button id to DX
  static const uint8_t _KP0_BTN_DX[KP0_NUM_KEYS];
  static const uint8_t _KP1_BTN_DX[KP1_NUM_KEYS];
  static const uint8_t _KP2_BTN_DX[KP2_NUM_KEYS];

  // Label to access all KP*_BTN_DX through indexes
  static const uint8_t *const _KP_BTN_DX[NUM_KP];

  // First DirectX value for encoders
  static const uint8_t _ENC_DX_BASE = 39;


  /************************/
  /* Protected data types */
  /************************/

  // Encoder direction normalized to be able to access arrays
  enum _EncDirArray: uint8_t
  {
    _EncDirACcw = 0U,
    _EncDirACw  = 1U
  };


  /*********************/
  /* Protected methods */
  /*********************/

  inline static uint8_t _encDx(uint8_t EncId, _EncDirArray EncDir);
};


#endif // _DX_H_
