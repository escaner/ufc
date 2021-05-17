#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdint.h>

/*
 *   Class to pass an input rotary encoder or keypad event.
 */

class Event
{
public:

  // Type of event
  enum EventId_t: uint8_t
  {
    EvNone = 0,
    EvKpPress,
    EvKpRelease,
    EvEncCcwPress,
    EvEncCcwRelease,
    EvEncCwPress,
    EvEncCwRelease
  };

  // Keypad press or release event
  struct KpEvent_t
  {
    uint8_t KpId;
    uint8_t KeyId;
  };

  // Rotary encoder turn event
/*
  struct EncEvent_t
  {
    uint8_t EncId;
    int8_t Value;
  };
*/

  /**********************/
  /* Public member data */
  /**********************/

  EventId_t Id;
  union
  {
    KpEvent_t Kp;
    uint8_t EncId;
/*      EncEvent_t Enc;*/
  };

  // Default constuctor
  inline Event();

  // Copy constructor and operator
  inline Event(const Event &Ev);
  inline Event &operator=(const Event &Ev);
};


/*
 *  Default constructor
 */
inline Event::Event(): Id(EvNone) {}


/*
 *  Copy constructor.
 */
inline Event::Event(const Event &Ev)
{
  Id = Ev.Id;

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

  switch (Id)
  {
  case EvKpPress:
  case EvKpRelease:
    Kp = Ev.Kp;
    break;
  case EvEncCcwPress:
  case EvEncCcwRelease:
  case EvEncCwPress:
  case EvEncCwRelease:
    EncId = Ev.EncId;
    break;
  }

#pragma GCC diagnostic pop
}


/*
 *   Copy operator.
 */
inline Event &Event::operator=(const Event &Ev)
{
  Id = Ev.Id;

#pragma GCC diagnostic push
  // Disable: warning: enumeration value '' not handled in switch
#pragma GCC diagnostic ignored "-Wswitch"

  switch (Id)
  {
  case EvKpPress:
  case EvKpRelease:
    Kp = Ev.Kp;
    break;
  case EvEncCcwPress:
  case EvEncCcwRelease:
  case EvEncCwPress:
  case EvEncCwRelease:
    EncId = Ev.EncId;
    break;
  }

#pragma GCC diagnostic pop

  return *this;
}


#endif // _EVENT_H_
