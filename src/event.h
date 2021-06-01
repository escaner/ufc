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


  /**********************/
  /* Public member data */
  /**********************/

  EventId_t Id;
  union
  {
    KpEvent_t Kp;
    uint8_t EncId;
  };

  // Default constuctor
  inline Event();

  // Copy constructor and operator
  inline Event(const Event &Ev);
  inline Event &operator=(const Event &Ev);
  inline bool isKp() const;
  inline bool isEnc() const;
  inline bool isEncCw() const;
  inline bool isEncCcw() const;
  inline bool isPress() const;
  inline bool isRelease() const;
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


/*
 *   Returns whether this is a keypad event.
 */
inline bool Event::isKp() const
{
  return Id == EvKpPress || Id == EvKpRelease;
}


/*
 *   Returns whether this is an encoder event.
 */
inline bool Event::isEnc() const
{
  return Id >= EvEncCcwPress && Id <= EvEncCwRelease;
}


/*
 *   Returns whether this is a clockwise encoder event.
 */
inline bool Event::isEncCw() const
{
  return Id == EvEncCwPress || Id == EvEncCwRelease;
}


/*
 *   Returns whether this is a counterclockwise encoder event.
 */
inline bool Event::isEncCcw() const
{
  return Id == EvEncCcwPress || Id == EvEncCcwRelease;
}


/*
 *   Returns whether this is a press event event.
 */
inline bool Event::isPress() const
{
  return Id==EvKpPress || Id==EvEncCwPress || Id==EvEncCcwPress;
}


/*
 *   Returns whether this is a release event event.
 */
inline bool Event::isRelease() const
{
  return Id==EvKpRelease || Id==EvEncCwRelease || Id==EvEncCcwRelease;
}


#endif // _EVENT_H_
