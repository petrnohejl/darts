#include "player.h"
#include <iostream>


/**
 * Player constructor, calls implicit constructor and sets the player's score value.
 */
Player::Player(int score) {
  darts = DARTS;
  this->score = score;
}

/**
 * Dart throwing simulation. In depend of situation returns action which is gonna be.
 * @param hit The hit number on disc.
 * @return NEXT, when next player is gonna play
 * END when this player finishes the game
 * CONT then this player continues throwing
 */
void Player::throwDart() {
  darts--;
}

/**
 * Sets player's score
 * @param s The score
 */
void Player::setScore(int s) {
  score = s;
}

/**
 * @return Player's score
 */
int Player::getScore() {
  return score;
}

/**
 * Sets player's darts count
 * @param d Count of darts
 */
void Player::setDarts(int d) {
  darts = d;
}

/**
 * @return Number of player's darts
 */
int Player::getDarts() {
  return darts;
}

