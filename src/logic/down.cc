#include <iostream>
#include "down.h"

/**
 * Constructor inherits Game constructor and initialize score to 501
 */
Down::Down(int players, int score) : Game(players, score) {
}

/**
 * Destructor, calls Game destructor
 */
Down::~Down() {
}

/**
 * Simulation of dart throwing
 * @param hit Value of hit number.
 */
enum action Down::throwDart(int hit){
  int oldScore = player[turn]->getScore();
  int score = oldScore-hit;
  player[turn]->throwDart();

  if (score < 0) {
    player[turn]->setScore(oldScore);
    //nextTurn();
    return NEXT;
  }

  player[turn]->setScore(score);

  if (score == 0) {
    winner = turn;
    return END;
  }

  if (player[turn]->getDarts() == 0) {
    return NEXT;
  }

  return CONT;
  
}

