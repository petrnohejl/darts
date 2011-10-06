#include "rtc.h"

/**
 * Constructor
 */
Rtc::Rtc(int players) : Game(players, 1) {
  
}

/**
 * Destructor
 */
Rtc::~Rtc() {
}

/**
 * Simulates dart throw by current player
 * @param hit Hit on target
 * @return Action that will be performed next
 */
enum action Rtc::throwDart(int hit) {
  int nextScore;
  int darts;

  player[turn]->throwDart();
  
  if (player[turn]->getScore() == hit) {
    nextScore = setNextTarget(hit);
    if (nextScore == 100) {
	  winner = turn;
      return END;
    }
    
    player[turn]->setScore(nextScore);
  }

  darts = player[turn]->getDarts();
  if (darts == 0) {
    return NEXT;
  }

  return CONT;
}

/**
 * Set's next target depending up hit value
 * @param previous The previous value of target
 * @return New target
 */
int Rtc::setNextTarget(int previous) {
  if (previous > 0 && previous < 20) {
    return (previous+1);
  }

  switch(previous) {
    case 20 : return 25;
      break;
    case 25 : return 50;
      break;
    case 50 : return 100;
      break;
    default : return -1;
  }
}
