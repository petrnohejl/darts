#include <iostream>
#include "game.h"

/**
 * Implicit constructor
 */
Game::Game() {
  player_count = 0;
}

/**
 * Creates array of pointers to players
 * @param players The number of players
 * @param score Initial player's score
 */
Game::Game(int players, int score) {
  winner = -1;
  player_count = players;
  player = new Player*[player_count];
  for(int i=0; i<player_count; i++) {
    player[i] = new Player(score);
  }
  turn = 0;
}

/**
 * Destructor, frees memmory of players
 */
Game::~Game() {
  for(int i=0; i<player_count; i++) {
    delete player[i];
  }

  delete[] player;
}

/**
 * Sets the number of next player's turn
 */
void Game::nextTurn() {
	player[turn]->setDarts(DARTS);
	turn++;
	turn %= player_count;
}

/**
 */
int Game::getPlayerScore(int idx) {
  return player[idx]->getScore();
}

/**
 */
int Game::getCurrentPlayer() {
  return turn;
}

/**
 *
 */
int Game::getPlayers() {
	return player_count;
}

/**
 * @return Winner's index
 */
int Game::getWinner() {
  return winner;
}

/**
 * @return Number of player darts
 */
int Game::getCurrentPlayerDarts() {
	return player[turn]->getDarts();
}


void Game::print() {
  std::cout << "Celkem je  " << player_count << " hracu." << std::endl;
  std::cout << "Na rade je hrac s indexem " << turn << std::endl;
  for(int i=0; i<player_count; i++) {
    std::cout << "\tHrac " << i << " ma score " << player[i]->getScore() << " a " << player[i]->getDarts() << " sipek." << std::endl;
  }
}
