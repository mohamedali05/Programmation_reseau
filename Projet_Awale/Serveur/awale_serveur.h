#ifndef AWALE_SERVEUR_H
#define AWALE_SERVEUR_H
#include "string.h"

#define LENGTH 12

void reset(int tab[], int points[], int value);
int is_zone(int player, int choice);
void eat(int tab[], int points[], int player, int cursor);
int is_finished(int tab[] , int points[] ) ; //returns 1 if the player 0 won , 2 if the player 1 won and 0 if the game isn't finished
void turn(int tab[], int points[], int player , int choice);
int move_allowed(int tab[] , int* choice , int player) ; 
//void gameLoop(int tab[], int points[]);
void print_table_to_char(int tab[], int points[],char* nom_joueur0, char* nom_joueur1 ,  char* output) ; 
void print_table(int tab[], int points[]);

#endif /* AWALE_H */