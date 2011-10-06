#include <iostream>
#include "down.h"
#include "rtc.h"
#include "training.h"

int main(int argc, char *argv[]) {
  Game *game;
  enum action act;
  int hit;

  game = new Down(3, 501);

  std::cout << "\n\n\tHraje se hra 501\n";
  do {
    std::cout << "Zadej trefu\n";
    std::cin >> hit;
    act = game->throwDart(hit);
    game->print();
  } while (act!=END);

  std::cout << "Vyhral hrac s cislem " << game->getWinner() << std::endl;
  
  delete game;

  game = new Rtc(2);

  std::cout << "\n\n\tHraje se hra kolotoc\n";
  do {
    std::cout << "Zadej trefu\n";
    std::cin >> hit;
    act = game->throwDart(hit);
    game->print();
  } while (act!=END && hit!=100);

  std::cout << "Vyhral hrac s cislem " << game->getWinner() << std::endl;

  delete game;

  game = new Training();

  std::cout << "\n\n\tHraje se trenink\n";
  do {
    std::cout << "Zadej trefu\n";
    std::cin >> hit;
    act = game->throwDart(hit);
    game->print();
  } while (hit!=100);

  std::cout << "Vyhral hrac s cislem " << game->getWinner() << std::endl;

  delete game;
  
  return 0;
}