#ifndef __DOWN_H__
#define __DOWN_H__

#include "game.h"

class Down : public Game {

  public:
    Down(int players, int score);
    ~Down();
    enum action throwDart(int hit);
};

#endif
