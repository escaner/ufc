#ifndef _MODE_H_
#define _MODE_H_

#include <Arduino.h>


/*
 *   Class to manage the mode of operation of the device.
 */
class Mode
{
public:

  // Define all the available modes of operation
  enum Id_t: uint8_t
  {
    M_DEFAULT = 0,
    M_A10C,
    M_F16,
    M_FA18C,
    M_M2000C,
    M_DEBUG,
    M_NUM_MODES
  };


  /******************/
  /* Member methods */
  /******************/

  Mode();
  Mode(uint8_t KeyId);
  Mode(const Mode &M);
  Mode &operator=(const Mode &M);
  inline Id_t get() const { return _Id; }
  const __FlashStringHelper *P_str() const;


protected:

  /********************/
  /* Static constants */
  /********************/

  static const int8_t _KEY[M_NUM_MODES] PROGMEM;
  static const char _TXT_DEFAULT[] PROGMEM;
  static const char _TXT_A10C[] PROGMEM;
  static const char _TXT_F16C[] PROGMEM;
  static const char _TXT_FA18C[] PROGMEM;
  static const char _TXT_M2000C[] PROGMEM;
  static const char _TXT_DEBUG[] PROGMEM;
  static const char * const _MODE_TXT[M_NUM_MODES] PROGMEM;


  /******************/
  /* Static methods */
  /******************/

  static Id_t _keyToMode(uint8_t KeyId);


  /***************/
  /* Member data */
  /***************/

  Id_t _Id;
};


#endif // _MODE_H_
