#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"


typedef struct Client
{
   SOCKET sock;
   char name[BUF_SIZE]; //pseudo
   char bio[2*BUF_SIZE]; //biographie
   struct Client* friends[MAX_FRIENDS]; //tableau des amis
   struct Client* pending_friend_request[MAX_PENDING_FRIEND_REQ]; //tableau des requêtes d'amis en attente
   int numFriends;
   int numPendingFriends;
   int isChallenged;
   int isPlaying;
   int isObserving;
   int private; //mode privé ou public
}Client;

#endif /* guard */
