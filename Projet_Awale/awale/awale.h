#ifndef AWALE_H
#define AWALE_H

#define LENGTH 12

void resetTable(int value);
void printTable();
int isZone(int player, int choice);
void eat(int player, int cursor);
void turn(int player);
void gameLoop();

#endif /* AWALE_H */