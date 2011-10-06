CC=g++
FLAGS=-std=c++98 -pedantic -Wall -O2 -w
LIBS=
PRG=darts-test

all: player.o game.o down.o rtc.o training.o main.o
	$(CC) $(LIBS) -o $(PRG) player.o game.o down.o rtc.o training.o main.o

player.o: player.cc
	$(CC) -c $(FLAGS) -o $@ $<

game.o: game.cc
	$(CC) -c $(FLAGS) -o $@ $<

main.o: main.cc
	$(CC) -c $(FLAGS) -o $@ $<

down.o: down.cc
	$(CC) -c $(FLAGS) -o $@ $<

rtc.o: rtc.cc
	$(CC) -c $(FLAGS) -o $@ $<

training.o: training.cc
	$(CC) -c $(FLAGS) -o $@ $<

clean: 
	rm -rf $(PRG) *o
