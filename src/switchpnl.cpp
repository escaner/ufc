#include "switchpnl.h"


/***************/
/* Static data */
/***************/

// KP0: analog value range (min, max) for each button in matrix; range [0, 1023]
const int16_t SwitchPnl::_KP0_KEY_RNG[KP0_NUM_KEYS][2] =
{
  { 1018, 1023 },  // 1023
  {  925,  935 },  //  930
  {  847,  857 },  //  852
  {  781,  791 },  //  786
  {  670,  680 },  //  675
  {  628,  638 },  //  633
  {  590,  600 },  //  595
  {  558,  568 },  //  563
  {  498,  508 },  //  503
  {  475,  485 },  //  480
  {  453,  463 },  //  458
  {  433,  443 },  //  438
  {  396,  406 },  //  401
  {  318,  328 },  //  323
  {  265,  275 },  //  270
  {  227,  237 }   //  232
};

// KP1: analog value range (min, max) for each button in matrix; range [0, 1023]
const int16_t SwitchPnl::_KP1_KEY_RNG[KP1_NUM_KEYS][2] =
{
  { 1018, 1023 },  // 1023 Left LSK1
  {  924,  934 },  //  929 CRS Right 
  {  845,  855 },  //  850 COM1
  {  766,  776 },  //  771 Left LSK2
  {  711,  721 },  //  716 CRS Left
  {  663,  673 },  //  668 HDG Right
  {  613,  623 },  //  618 Left LSK3
  {  577,  587 },  //  582 COM2
  {  545,  555 },  //  550 HDG Left
  {  511,  521 },  //  516 Left LSK4
  {  486,  496 }   //  491 Master Warning 
};

// KP2: analog value range (min, max) for each button in matrix; range [0, 1023]
const int16_t SwitchPnl::_KP2_KEY_RNG[KP2_NUM_KEYS][2] =
{
  { 1018, 1023 },  // 1023 Right LSK1
  {  923,  933 },  //  928 Encoder left (AMPCD OBS RNG)
  {  845,  855 },  //  850 Encoder right (AMPCD OBS SEQ)
  {  766,  776 },  //  771 Right LSK2
  {  711,  721 },  //  716 Button (AMPCD OBS TCN)
  {  663,  673 },  //  668 Button (AMPCD OBS AUTO)
  {  613,  623 },  //  618 Right LSK3
  {  577,  587 },  //  582 Button (AMPCD OBS TCN)
  {  545,  555 },  //  550 Button (AMPCD OBS WPT)
  {  512,  522 },  //  517 Right LSK4
  {  486,  496 },  //  491 Button (AMPCD OBS MK)
  {  464,  474 }   //  469 Button (AMPCD OBS WPDSG)
};

// Label to access all KP*_NUM_KEYS through indexes
const uint8_t SwitchPnl::_KP_NUM_KEYS[NUM_KP] =
{
  KP0_NUM_KEYS,
  KP1_NUM_KEYS,
  KP2_NUM_KEYS
};

// Label to access all KP*_KEY_RNG through indexes
const int16_t (* const SwitchPnl::_KP_KEY_RNG[NUM_KP])[2] =
{
  _KP0_KEY_RNG,
  _KP1_KEY_RNG,
  _KP2_KEY_RNG
};


/******************/
/* Member methods */
/******************/

/*
 *   Constructor: initializes member data.
 *  The button values are pulled HIGH therefore that will be considered their
 *  initial state, being LOW when pressed.
 *  Parameters:
 *  * KpPin: Arduino pins where the analog keypads are connected
 *  * EncPin: Arduino pins where the encoder pins are connected
 */
SwitchPnl::SwitchPnl(const uint8_t KpPin[NUM_KP],
    const uint8_t EncPin[NUM_ENC][ENC_NUM_PINS]):
  _KpPin(KpPin),
  _EncPin(EncPin),
  _Keypad{SwitchKp(SwitchKp::SWITCH_NONE, KP_DEBOUNCE_TIME),
    SwitchKp(SwitchKp::SWITCH_NONE, KP_DEBOUNCE_TIME),
    SwitchKp(SwitchKp::SWITCH_NONE, KP_DEBOUNCE_TIME)},
  _Encoder{REncoderAsync(DX_DELAY_PR, DX_DELAY_RP),
    REncoderAsync(DX_DELAY_PR, DX_DELAY_RP),
    REncoderAsync(DX_DELAY_PR, DX_DELAY_RP),
    REncoderAsync(DX_DELAY_PR, DX_DELAY_RP)}
{
}


/*
 *   Configures Arduino pins for switch panel keypads and encoders, pulling
 *  them up as required.
 *  Parameters:
 *  * Keypad: in which keypad to look for a pressed button
 *  Returns: key id in Keypad that is being depressed during initialization
 *  or SwitchKp::SWITCH_NONE when none is.
 */
 uint8_t SwitchPnl::init(uint8_t Keypad) const
{
  uint8_t KpId, EncId, EncPinId;

  // Initialize Keypad pins
  for (KpId = 0U; KpId < NUM_KP; KpId++)
    pinMode(_KpPin[KpId], INPUT);

  // Initialize encoder pins
  for (EncId = 0U; EncId < NUM_ENC; EncId++)
    for (EncPinId = 0U; EncPinId < ENC_NUM_PINS; EncPinId++)
      pinMode(_EncPin[EncId][EncPinId], INPUT_PULLUP);

  // Read value on Keypad and translate to its key
  return _kpGetKey(Keypad);
}


/*
 *   Sets the delays for the DirecX events of one of the encoders.
 *  Parameters:
 *  * EncId: encoder identifier
 *  * DelayPR: delay from press to release in ms.
 *  * DelayRP: delay from release to press in ms.
 */
void SwitchPnl::setEncDelay(EncId_t EncId, uint8_t DelayPR, uint8_t DelayRP)
{
  _Encoder[EncId].setDelayPressRel(DelayPR);
  _Encoder[EncId].setDelayRelPress(DelayRP);
}


/*
 *   Once init() has finished, this method waits until Keypad returns back to
 *  SwitchKp::SWITCH_NONE before returning.
 */
void SwitchPnl::wait(uint8_t Keypad) const
{
  // Active wait until the key is released
  while (_kpGetKey(Keypad) != SwitchKp::SWITCH_NONE)
    ;
}


/*
 *   Reads status of keypads and encoders and returns an Event when changes are
 *  detected. As it is needed for the switch readings to stabilize, one single
 *  read with a change will never deliver a switch change event. Therefore
 *  the LoopCnt parameter is provided to try a maximum of LoopCnt iterations
 *  before giving up and returning a no event status. The moment a switch change
 *  is positively registered, the method just returns it.
 *  Parameters:
 *  * LoopCnt: loop a maximum of LoopCnt iterations waiting for the switch
 *    to stabilize.
 *  Returns: the registered switch event if any, otherwise SwEvNone.
 */
Event SwitchPnl::check(uint16_t LoopCnt)
{
  Event Ev;
  uint8_t Id, KeyId, EventKeyId;
  int8_t EventEnc;
  uint8_t EncValA, EncValB;
  SwitchKp::Flank_t Flank;

  // Iterate LoopCnt times waiting for the switch event to stabilize
  // As soon as an event is detected, it is returned
  while (LoopCnt--)
  {
    // Update all rotary encoders
    for (Id = 0U; Id < NUM_ENC; Id++)
    {
      // Read the encoder values
      EncValA = digitalRead(_EncPin[Id][0]);
      EncValB = digitalRead(_EncPin[Id][1]);

      // Update encoder status
      _Encoder[Id].update(EncValA, EncValB);
    }

    // Check for pending rotary encoder events, they are asynchronous
    for (Id = 0U; Id < NUM_ENC; Id++)
    {
      EventEnc = _Encoder[Id].getEvent();

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

      switch (EventEnc)
      {
      case REncoderAsync::EV_CCW_PRESS:
        Ev.Id = Event::EvEncCcwPress;
        Ev.EncId = Id;
        return Ev;
      case REncoderAsync::EV_CCW_RELEASE:
        Ev.Id = Event::EvEncCcwRelease;
        Ev.EncId = Id;
        return Ev;
      case REncoderAsync::EV_CW_PRESS:
        Ev.Id = Event::EvEncCwPress;
        Ev.EncId = Id;
        return Ev;
      case REncoderAsync::EV_CW_RELEASE:
        Ev.Id = Event::EvEncCwRelease;
        Ev.EncId = Id;
        return Ev;
      // default: EV_NONE -> do nothing
      }

#pragma GCC diagnostic pop

    }

    // Check keypads
    for (Id = 0U; Id < NUM_KP; Id++)
    {
      // Get the key read and check for flanks
      KeyId = _kpGetKey(Id);
      Flank = _Keypad[Id].updateFlank(KeyId, &EventKeyId);

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

      switch (Flank)
      {
      case SwitchKp::FLANK_RISING:
        // Key pressed -> return data
        Ev.Id = Event::EvKpPress;
        Ev.Kp.KpId = Id;
        Ev.Kp.KeyId = EventKeyId;
        return Ev;
      case SwitchKp::FLANK_FALLING:
        // Key released -> return data
        Ev.Id = Event::EvKpRelease;
        Ev.Kp.KpId = Id;
        Ev.Kp.KeyId = EventKeyId;
        return Ev;
      // default: FLANK_NONE -> do nothing
      }

#pragma GCC diagnostic pop

    }
  }
 
  return Ev;
}


/*
 *   Converts the analog value read from the Keypad to the corresponding KeyId.
 *  Note that the array is sorted with lower values at the end. No key pressed
 *  will be aprox. value 0, so this is the first that will be checked.
 *  Parameters:
 *  * KpId: keypad where KpVal was read
 *  * KpVal: analog value for the keypad read in the Arduino pin
 *  Returns:
 *   key matching the corresponding analog KpVal or SwitchKp::SWITCH_NONE when
 *   none matches.
 */
uint8_t SwitchPnl::_kpValueToKey(uint8_t KpId, int16_t KpVal) const
{
  uint8_t KeyId;
  const int16_t (*KeyRng)[2];

  // Start checking from the higher KeyId minimum value as this will identify
  // quick when no key is pressed.
  KeyRng = _KP_KEY_RNG[KpId];
  KeyId = _KP_NUM_KEYS[KpId];
  while (KeyId-- != 0U)
  {
    // If value lower than minimum -> no valid key pressed
    if (KpVal < KeyRng[KeyId][0])
      return SwitchKp::SWITCH_NONE;

    // Now, value >= minimum for this key

    // If value is <= than maximum for this key, we found the key
    if (KpVal <= KeyRng[KeyId][1])
      break;

    // Otherwise, keep looping
  }
  // Note that maximum value should be included in the array, and never reach
  // underflow in KeyId, but still it will reset to UINT8_MAX, which is the
  // value for SwitchKp::SWITCH_NONE

  return KeyId;
}
