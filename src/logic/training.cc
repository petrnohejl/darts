#include "training.h"

/**
 *
 */
Training::Training() : Game (1,0) {
  player[turn]->setDarts(1);
}

/**
 *
 */
Training::~Training() {
}

/**
 *
 */
enum action Training::throwDart(int hit) {
  player[turn]->setScore(hit);
  return CONT;
}
