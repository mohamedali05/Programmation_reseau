#include <stdio.h>
#include "awale_serveur.h"

void reset(int tab[], int points[], int value)
{
    for (int i = 0; i < LENGTH; i++)
    {
        tab[i] = value;
    }
}
void printTableToChar(int tab[], int points[] , char* output)
{
    // Initialize the string
    output[0] = '\0';

    // 11 | 10 | 9 | 8 | 7 | 6  <- player 1
    //-------------------------
    // 0  | 1  | 2 | 3 | 4 | 5  <- player 0
    int pipeCount = 0;

    // Format the output string
    for (int i = LENGTH - 1; i >= LENGTH / 2; i--)
    {
        snprintf(output + strlen(output), 256 - strlen(output), "%d", tab[i]);
        if (pipeCount < LENGTH / 2 - 1)
        {
            snprintf(output + strlen(output), 256 - strlen(output), " | ");
        }
        pipeCount++;
    }

    snprintf(output + strlen(output), 256 - strlen(output), "  Player 1: %d\n", points[1]);
    pipeCount = 0;

    for (int i = 0; i < LENGTH / 2; i++)
    {
        snprintf(output + strlen(output), 256 - strlen(output), "%d", tab[i]);
        if (pipeCount < LENGTH / 2 - 1)
        {
            snprintf(output + strlen(output), 256 - strlen(output), " | ");
        }
        pipeCount++;
    }

    snprintf(output + strlen(output), 256 - strlen(output), "  Player 0: %d\n", points[0]);

}

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

int isZone(int player, int choice)
{
    return choice / (LENGTH / 2) == player;
}

void eat(int tab[], int points[], int player, int cursor)
{
    while (1)
    {
        if (!isZone(player, cursor) && (tab[cursor] == 2 || tab[cursor] == 3))
        {
            points[player] += tab[cursor];
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

int moveAllowed(int tab[] , int* choice , int player){
    //0 if the move isn't allowed and 1 if the move is allowed
        if ((*choice) < LENGTH / 2 + 1 && (*choice) > 0){
        
            (*choice) = (*choice) - 1 ;
            if (player == 1)
            {
                (*choice) = LENGTH - 1 - (*choice);
            }
            if (tab[(*choice)] > 0){
                return 1 ; 
            }
                
        }
        return 0  ; 
}
    

void turn(int tab[], int points[], int player, int choice){
    int cursor = choice;
    while(1){
        if (tab[choice] == 0){
                break;
        }
        cursor++;
        if (cursor == LENGTH){
            cursor = 0;
        }
        if (cursor == choice){
            cursor++;
        }
            tab[choice]--;
            tab[cursor]++;
    }
    eat(tab , points , player, cursor); 
}
    

        
    


/*void gameLoop(int tab[], int points[])
{
    reset(tab, points, 4);
    while (1)
    {
        // Send Table
        turn(tab, points, 0);
        // Send Table
        turn(tab, points, 1);
    }
}
/*
int main()
{
    int tab[LENGTH];
    int points[2];
    gameLoop(tab, points);
    return 0;
}
*/
