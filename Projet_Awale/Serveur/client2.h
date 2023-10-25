#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"


typedef struct Client
{
   SOCKET sock;
   char name[BUF_SIZE];
   char bio[2*BUF_SIZE];
   struct Client* friends[MAX_FRIENDS];
   struct Client* pending_friend_request[MAX_PENDING_FRIEND_REQ];
   int numFriends;
   int numPendingFriends;
   int isChallenged;
   int isPlaying;
   int isObserving;
   int private;
}Client;

#endif /* guard */
