#ifndef __TRAINING_H__
#define __TRAINING_H__

#include "game.h"

class Training : public Game {

  public:
    Training();
    ~Training();
    enum action throwDart(int hit);
};

#endif
