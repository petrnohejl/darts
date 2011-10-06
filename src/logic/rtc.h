#ifndef __RTC_H__
#define __RTC_H__

#include "game.h"

class Rtc : public Game {
  
  public:
    Rtc(int players);
    ~Rtc();
    enum action throwDart(int hit);

  private:
    int setNextTarget(int previous);
};

#endif
