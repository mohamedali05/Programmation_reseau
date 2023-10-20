#ifndef AWALE_SERVEUR_H
#define AWALE_SERVEUR_H

#define LENGTH 12

void reset(int tab[], int points[], int value);
int isZone(int player, int choice);
void eat(int tab[], int points[], int player, int cursor);
void turn(int tab[], int points[], int player);
void gameLoop(int tab[], int points[]);
void printTable(int tab[], int points[]);

#endif /* AWALE_H */