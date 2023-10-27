#include <stdio.h>
#include "awale_serveur.h"

void reset(int tab[], int points[], int value)
{
    for (int i = 0; i < LENGTH; i++)
    {
        tab[i] = value;
    }
}
void print_table_to_char(int tab[], int points[], char* nom_joueur0, char* nom_joueur1  , char* output)
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

    snprintf(output + strlen(output), 256 - strlen(output), "  %s : %d points\n",nom_joueur1 ,  points[1]);
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

    snprintf(output + strlen(output), 256 - strlen(output), "  %s : %d points\n", nom_joueur0  ,points[0]);

}

void print_table(int tab[], int points[])
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

int is_zone(int player, int choice)
{
    return choice / (LENGTH / 2) == player;
}
int is_finished(int tab[] , int points[] ){
    if (points[0]> 1){
        return 1 ; 
    }
    if (points[1]> 1){
        return 2 ; 
    }
    return 0 ; 
    
}

void eat(int tab[], int points[], int player, int cursor)
{
    while (1)
    {
        if (!is_zone(player, cursor) && (tab[cursor] == 2 || tab[cursor] == 3))
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

int move_allowed(int tab[] , int* choice , int player){
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
    eat(tab, points, player, cursor); 
}