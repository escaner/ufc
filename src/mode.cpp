#include "mode.h"
#include <SwitchKp.h>


/********************/
/* Static constants */
/********************/

// For each Mode_t (index), which key (value) selects it
const int8_t Mode::_KEY[M_NUM_MODES] PROGMEM =
{
  15, // M_DIRECTX -> CLR
  11, // M_A10C -> 0
   5, // M_F16C -> 6
  10, // M_FA18C -> 8
   8, // M_M2000C -> 2
   3  // M_DEBUG -> ON
};

// Strings with the names of the modes
const char Mode::_TXT_DIRECTX[] PROGMEM = "DIRECTX";
const char Mode::_TXT_A10C[] PROGMEM = "A-10C";
const char Mode::_TXT_F16C[] PROGMEM = "F-16C";
const char Mode::_TXT_FA18C[] PROGMEM = "F/A-18C";
const char Mode::_TXT_M2000C[] PROGMEM = "MIRAGE 2000C";
const char Mode::_TXT_DEBUG[] PROGMEM = "DEBUG";
const char * const Mode::_MODE_TXT[M_NUM_MODES] PROGMEM =
{
  _TXT_DIRECTX,
  _TXT_A10C,
  _TXT_F16C,
  _TXT_FA18C,
  _TXT_M2000C,
  _TXT_DEBUG
};


/******************/
/* Member methods */
/******************/


/*
 *   Constructor. Detaults to M_DIRECTX mode.
 */
Mode::Mode():
  _Id(M_DIRECTX)
{
}


/*
 *   Constructor.
 *   Parameters:
 *   * KeyId: identifier of the key that was pressed. It can be SWITCH_NONE.
 */
Mode::Mode(uint8_t KeyId):
  _Id(_keyToMode(KeyId))
{
}


/*
 *   Copy constructor.
 */
Mode::Mode(const Mode &M):
  _Id(M._Id)
{
}


/*
 *   Assignment operator.
 */
Mode &Mode::operator=(const Mode &M)
{
  _Id = M._Id;
  return *this;
}

/*
 *   Sets the mode of operation to Id. If it is not valid, the M_DEFAULT mode
 *  is set instead.
 *   Parameters:
 *   * Id: the new mode of operation
 */
void Mode::set(Id_t Id)
{
  // Is Id a valid mode?
  if (Id < M_NUM_MODES)
    // Yes
    _Id = Id;
  else
    // No: use default mode
    _Id = M_DEFAULT;
}


/*
 *   Returns a string in program memory with the mode name.
 */
const __FlashStringHelper *Mode::P_str() const
{
  return (const __FlashStringHelper *) pgm_read_word(_MODE_TXT + _Id);
}



/******************/
/* Static methods */
/******************/

/*
 *   Looks for which mode corresponds to the given key identifier. When none
 *  found, reverts to M_DIRECTX mode.
 *   Parameters:
 *   * KeyId: key identifier that possibly corresponds to a mode. It can be
 *            SWITCH_NONE
 *   Returns: the mode that corresponds to KeyId, or M_DIRECTX when none defined
 */
Mode::Id_t Mode::_keyToMode(uint8_t KeyId)
{
  uint8_t ModeId;

  if (KeyId != SwitchKp::SWITCH_NONE)
    // Look for the pressed key in the table _KEY
    for (ModeId = 0U; ModeId < M_NUM_MODES; ModeId++)
      if (_KEY[ModeId] == KeyId)
        return (Id_t) ModeId;

  // If not found or no key pressed, return default mode
  return M_DEFAULT;
}
