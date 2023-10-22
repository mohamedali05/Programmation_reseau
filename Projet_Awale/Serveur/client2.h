#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   char bio[2*BUF_SIZE] ; 
   int isChallenged;
   int isPlaying ; 
}Client;

#endif /* guard */
