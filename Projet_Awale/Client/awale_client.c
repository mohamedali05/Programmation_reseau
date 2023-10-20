#include <stdio.h>
#include "awale_client.h"

void printTable(int tab[], int points[])
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
    printf("  Player 1: %d\n", points[1]);
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
    printf("  Player 0: %d\n", points[0]);
}