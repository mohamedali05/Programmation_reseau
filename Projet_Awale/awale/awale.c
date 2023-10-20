#include <stdio.h>
#include "awale.h"

int tab[LENGTH];
int point[2];

void resetTable(int value)
{
    for (int i = 0; i < LENGTH; i++)
    {
        tab[i] = value;
    }
}

void printTable()
{
    // 11 | 10 | 9 | 8 | 7 | 6  <- player 1
    //-------------------------
    // 0  | 1  | 2 | 3 | 4 | 5  <- player 0
    int pipeCount = 0;
    for (int i = LENGTH - 1; i >= LENGTH / 2; i--)
    {
        printf("%d", tab[i]);
        if (pipeCount < LENGTH / 2 - 1)
        {
            printf(" | ");
        }
        pipeCount++;
    }
    printf("  Player 1: %d\n", point[1]);
    pipeCount = 0;
    for (int i = 0; i < LENGTH / 2; i++)
    {
        printf("%d", tab[i]);
        if (pipeCount < LENGTH / 2 - 1)
        {
            printf(" | ");
        }
        pipeCount++;
    }
    printf("  Player 0: %d\n", point[0]);
}

int isZone(int player, int choice)
{
    return choice / (LENGTH / 2) == player;
}

void eat(int player, int cursor)
{
    while (1)
    {
        if (!isZone(player, cursor) && (tab[cursor] == 2 || tab[cursor] == 3))
        {
            point[player] += tab[cursor];
            tab[cursor] = 0;
        }
        else
        {
            break;
        }
        cursor--;
        if (cursor < 0)
        {
            break;
        }
    }
}

void turn(int player)
{
    int choice;
    while (1)
    {
        printf("Enter your move from 1-6, Player %d: ", player);
        scanf("%d", &choice);
        if (choice < LENGTH / 2 + 1 && choice > 0)
        {
            choice--;
            if (player == 1)
            {
                choice = LENGTH - 1 - choice;
            }
            if (tab[choice] > 0)
                break;
        }
        printf("Illegal move. Choose again.\n"); 
    }
    int cursor = choice;
    while (1)
    {
        if (tab[choice] == 0)
        {
            break;
        }
        cursor++;
        if (cursor == LENGTH)
        {
            cursor = 0;
        }
        if (cursor == choice)
        {
            continue;
        }
        tab[choice]--;
        tab[cursor]++;
    }
    eat(player, cursor);
}

void gameLoop()
{
    resetTable(4);
    while (1)
    {
        printTable();
        turn(0);
        printTable();
        turn(1);
    }
}

int main()
{
    gameLoop();
    return 0;
}