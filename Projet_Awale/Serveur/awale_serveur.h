#ifndef AWALE_SERVEUR_H
#define AWALE_SERVEUR_H
#include "string.h"

#define LENGTH 12

void reset(int tab[], int points[], int value);
int isZone(int player, int choice);
void eat(int tab[], int points[], int player, int cursor);
void turn(int tab[], int points[], int player , int choice);
int moveAllowed(int tab[] , int* choice , int player) ; 
//void gameLoop(int tab[], int points[]);
void printTableToChar(int tab[], int points[], char* output) ; 
void printTable(int tab[], int points[]);

#endif /* AWALE_H */