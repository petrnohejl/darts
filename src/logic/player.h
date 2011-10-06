/**
 *
 */

#ifndef __PLAYER_H__
#define __PLAYER_H__

#define DARTS 3

///Future action that will follow the next throw
enum action {CONT, NEXT, END};

class Player {
  ///Player's score
  int score;
  ///Count of player's dart to throw
  short darts;

  public:
//     Player();
    Player(int score=0);
    void throwDart();
    void setScore(int s);
    int getScore();
    void setDarts(int d);
    int getDarts();
};

#endif
