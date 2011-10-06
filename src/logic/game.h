/**
 * Modul contains the main game class declarating virtual methods for generic game type.
 */

#ifndef __GAME_H__
#define __GAME_H__

#include "player.h"

class Game {
  protected:
    ///Count of players
    int player_count;
    ///Pointer on player
    Player **player;
    ///Player that is on turn
    int turn;
    ///Player that has won the game
    int winner;
  
  public:
//     virtual void init() = 0;
    virtual enum action throwDart(int hit) = 0;
    void nextTurn();
    int getWinner();
	int getCurrentPlayerDarts();
	int getPlayerScore(int idx);
	int getPlayers();
	int getCurrentPlayer();
    void print();
    Game();
    Game(int players, int score);
    ~Game();
};

#endif
