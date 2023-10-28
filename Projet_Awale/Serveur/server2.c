#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"
#include "awale_serveur.h"

Challenge challenges[MAX_CLIENTS];
int total_challenges = 0;

 void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

 void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

 void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   // list of all commands for the players
   Commands commandList[] = {
    {"/challenge [pseudo]", "Défier [pseudo] pour une partie"},
    {"/ready", "Accepter un challenge en attente"},
    {"/refuse", "Refuser un challenge en attente"},

    {"/add_friend [pseudo]", "Envoyer une requête d'ami à [pseudo]"},
    {"/unfriend [pseudo]", "Retirer [pseudo] de vos amis"},
    {"/accept [pseudo]", "Accepter la requête d'ami de [pseudo]"},
    {"/decline [pseudo]", "Refuser la requête d'ami de [pseudo]"},

    {"/discuss [pseudo] [text]", "Envoyer le texte [text] à [pseudo]"},
    {"/discuss1 [text]", "Envoyer le texte [text] au joueur adverse lors d'une partie"},

    {"/define_bio [your_bio]", "Définir sa biographie avec [your_bio]"},
    {"/view_bio [pseudo]", "Voir la biographie de [pseudo]"},

    {"/view_matches", "Voir les matchs en cours"},
    {"/spectate [id_match]", "Spectate le match [id]"},
    {"/stop", "Arrêter de spectate un match"},

    {"/public", "Passer en mode public"},
    {"/private", "Passer en mode privé"},

    {"/list_players", "Voir la liste des joueurs en ligne"},
    {"/list_friends", "Voir la liste de ses amis disponibles"},
    {"/list_pending", "Voir la liste des requêtes d'ami en attente"},
    {"/list_commands", "Voir la liste des commandes disponibles"}
   };

   char infos_commands[1000];
   strcpy(infos_commands, "Liste des commandes :\n");
   
   for (int i = 0; i < sizeof(commandList) / sizeof(commandList[0]); i++) {
      strcat(infos_commands, commandList[i].command);
      strcat(infos_commands, " : ");
      strcat(infos_commands, commandList[i].description);
      strcat(infos_commands, "\n");
   }

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = { csock };
         strncpy(c.name, buffer, BUF_SIZE - 1);
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
        {

            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);  
               /* client disconnected */ 
               if(c == 0)
               {
                  int numChallenge = find_challenge_By_player_for_disconnection(clients[i]) ;

                  if(numChallenge != -1) {

                     challenges[numChallenge].Client_disconnected = 1 ;
   
                     if (strcmp(clients[i].name ,challenges[numChallenge].challenged->name) == 0){
                        if (challenges[numChallenge].state == -1){
                           write_client(challenges[numChallenge].challenger->sock , "Le client que vous avez challengé s'est déconnecté\n") ;
                           
                        }else if (challenges[numChallenge].state == 1){
                           write_client(challenges[numChallenge].challenger->sock , "Le client avec lequel vous jouez s'est déconnecté\n") ;
                           challenges[numChallenge].challenger->isPlaying = 0 ; 
                           
                        }
                     }else{
                        if (challenges[numChallenge].state == -1){
                           write_client(challenges[numChallenge].challenged->sock , "Le client qui vous a challengé s'est déconnecté\n") ; 
                           challenges[numChallenge].challenged->isChallenged = 0 ;
                           
                        }else if (challenges[numChallenge].state == 1){
                           write_client(challenges[numChallenge].challenged->sock , "Le client avec lequel vous jouez s'est déconnecté\n") ;
                           challenges[numChallenge].challenged->isPlaying = 0 ;
                        }
                     }

                     challenges[numChallenge].state = 2;

                  }
                  
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
                  if (strcmp(buffer,"/list_commands") == 0 ){
                     write_client(clients[i].sock, infos_commands);
                  } else if (strcmp(buffer, "/list_players") == 0) {
                     handle_list_request(&clients[i], clients, actual);
                  } else if (strcmp(buffer, "/list_friends") == 0){
                     handle_friend_list(&clients[i]);
                  } else if (strcmp(buffer, "/list_pending") == 0){
                     handle_pending_friend_list(&clients[i]);
                  }else if ((strstr(buffer, "/define_bio") != NULL) ){
                     define_bio(&clients[i] , buffer) ; 
                  }else if(strstr(buffer, "/view_bio")!= NULL){
                     view_bio(&clients[i] ,clients , actual,   buffer) ;
                  }else if(strcmp(buffer, "/view_matches") == 0){
                     view_list_matches(&clients[i]) ; 
                  } else if ((strstr(buffer, "/challenge") != NULL)) {
                     handle_challenge_request(&clients[i], clients, actual, buffer);
                  } else if(strcmp(buffer, "/ready") == 0 && clients[i].isChallenged) {
                     accept_challenge_request(&clients[i], clients, actual);
                  } else if(strcmp(buffer, "/refuse") == 0 && clients[i].isChallenged) {
                     refuse_challenge_request(&clients[i]);
                  } else if ((strstr(buffer, "/add_friend") != NULL)) {
                     handle_friend_request(&clients[i], clients, actual, buffer);
                  } else if((strstr(buffer, "/accept") != NULL)) {
                     accept_friend_request(&clients[i], clients, actual, buffer);
                  } else if((strstr(buffer, "/decline") != NULL)) {
                     decline_friend_request(&clients[i], clients, actual, buffer);
                  } else if((strstr(buffer, "/unfriend") != NULL)) {
                     unfriend_friend(&clients[i], clients, actual, buffer);
                  } else if (clients[i].isPlaying && est_nombre(buffer)){
                     handle_game(&clients[i], buffer) ; 
                  } else if(clients[i].isPlaying && (strstr(buffer, "/discuss1") != NULL)){
                     handle_discussion1(&clients[i], buffer) ; 
                  } else if(clients[i].isPlaying && strcmp(buffer, "/forfait") == 0){
                     handle_forfait(&clients[i], buffer) ; 
                  }else if((strstr(buffer, "/discuss1") == NULL) && (strstr(buffer, "/discuss") != NULL)){
                     handle_discussion(&clients[i], buffer, clients, actual);
                  } else if((strstr(buffer, "/spectate") != NULL) && !clients[i].isObserving && !clients[i].isPlaying) {
                     observe_match(&clients[i], buffer);
                  } else if(strcmp(buffer, "/stop") == 0 && clients[i].isObserving){
                     stop_observe(&clients[i]);
                  } else if(strcmp(buffer, "/private") == 0){
                     switch_private(&clients[i]);
                  } else if(strcmp(buffer, "/public") == 0){
                     switch_public(&clients[i]);
                  } else {
                     write_client(clients[i].sock, "La commande est incorrecte");
                  }
               }
               
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

 void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

 void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

 void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

 int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()") ;
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

 void end_connection(int sock)
{
   closesocket(sock);
}

 int read_client(SOCKET sock, char *buffer)
{
   int n = 0;


   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

void write_client(SOCKET sock, const char *buffer)
{
  if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

/*Permet d'afficher la liste des joueurs*/
void handle_list_request(Client* sender, Client *clients, int actual) {
    char response[BUF_SIZE];
    response[0] = 0;
    for (int i = 0; i < actual; i++) {
        strncat(response, clients[i].name, BUF_SIZE - 1);
        if (strlen(clients[i].bio) > 0) {
            strncat(response, " - Bio: ", BUF_SIZE - strlen(response) - 1);
            strncat(response, clients[i].bio, BUF_SIZE - strlen(response) - 1);
        }
        if (i != actual -1) {
            strncat(response, "\n", BUF_SIZE - strlen(response) - 1);
        }
    }
   write_client(sender->sock, "Liste des joueurs disponibles :\n");
   write_client(sender->sock, response);
}

/*Permet d'afficher la liste des amis du joueur*/
void handle_friend_list(Client* sender) {
   char response[BUF_SIZE];
   response[0] = 0;

   if (sender->numFriends > 0) {
        strcat(response, "Liste de vos amis :\n");
        for (int i = 0; i < sender->numFriends; ++i) {
            strcat(response, sender->friends[i]->name);
            strcat(response, "\n");
        }
   } else {
        strcat(response, "Vous n'avez pas d'amis pour le moment.\n");
   }
   write_client(sender->sock, response); 
}

/*Permet d'afficher la liste des requêtes d'ami en attente du joueur*/
void handle_pending_friend_list(Client* sender) {
   char response[BUF_SIZE];
   response[0] = 0;
   if (sender->numPendingFriends > 0) {
        strcat(response, "Liste des joueurs qui vous ont demandé en ami :\n");
        for (int i = 0; i < sender->numPendingFriends; ++i) {
            strcat(response, sender->pending_friend_request[i]->name);
            strcat(response, "\n");
        }
   } else {
        strcat(response, "Vous n'avez pas de requête d'ami pour le moment.\n");
   }
   write_client(sender->sock, response); 
}

/*Permet de définir la biographie d'un joueur*/
void define_bio(Client* sender ,char*buffer){
   char* bio_start = buffer + strlen("/define_bio ") ;
   strcpy(sender->bio, bio_start) ; 
   write_client(sender->sock ,"Votre bio a bien été enregistré") ; 
}

/*Permet d'afficher la biographie d'un joueur*/
void view_bio(Client* sender , Client* clients ,int actual , char* buffer){
   Client *target = extract_target_by_name(clients, buffer+strlen("/view_bio") +1, actual);
   if (target == NULL){
      write_client(sender->sock ,"Ce joueur n'existe pas") ; 
      return ; 
   }
   write_client(sender->sock,target->bio) ; 
}

/*Permet d'afficher la liste des matchs en cours*/
void view_list_matches(Client* sender){
   char affichage[BUF_SIZE] = "";
   int foundMatch = 0;
   for (int i = 0 ; i<total_challenges ; i++){
      if (challenges[i].state == 1) {
         foundMatch = 1;
         char temp[BUF_SIZE] ; 
         snprintf(temp, BUF_SIZE, "%d : %s vs %s \n", i + 1, challenges[i].challenged->name, challenges[i].challenger->name);
         strncat(affichage, temp, BUF_SIZE - strlen(affichage) - 1);
      }
   }
   if (!foundMatch){
      write_client(sender->sock, "Il n'y a pas de match en cours");
   } else {
      write_client(sender->sock, affichage);
   }
}

/*Permet de gérer les demandes de challenge*/
void handle_challenge_request(Client* sender, Client *clients, int actual, const char *buffer){
   Client *target = extract_target_by_name(clients, buffer +strlen("/challenge") +1 , actual);
   if (target == NULL){
      write_client(sender->sock ,"Ce joueur n'existe pas") ; 
      return ; 
   }
   if (target->isPlaying){
      write_client(sender->sock ,"Ce joueur est déjà en train de jouer") ; 
      return ; 
   }
   if(target == sender){
      write_client(sender->sock ,"Vous ne pouvez pas vous challenger vous même") ;
      return ; 
   }
   if(target->isChallenged){
      write_client(sender->sock ,"Cette personne a déjà été challengée ; attendez qu'elle réponde d'abord à ce challenge") ;
      return ; 
   }

   challenges[total_challenges].challenger = sender ; 
   challenges[total_challenges].challenged = target ;    
   challenges[total_challenges].state = -1;  // -1 indique en attente d'une réponse.
   challenges[total_challenges].nbObservers = 0;
   target->isChallenged = 1;

   char invitation[BUF_SIZE];
   strcpy(invitation , sender->name) ; 
   strcat(invitation , " vous a défié.\n/ready ou /refuse pour répondre.") ; 
   write_client(challenges[total_challenges].challenged->sock, invitation);
   write_client(challenges[total_challenges].challenger->sock , "Challenge envoyé") ; 
   total_challenges++;
}

/*Permet d'obtenir un client par son nom*/
Client* extract_target_by_name(Client* clients , const char* buffer, int actual){
   for (int i = 0; i < actual; i++) {
         char *name = clients[i].name;
        if (strcmp(buffer, name) == 0) {
            return &(clients[i]);
        }
    }
    return NULL;
}

/*Permet de gérer l'acceptation d'un challenge*/
void accept_challenge_request(Client* sender , Client* Clients ,   int actual){
   char affichage[BUF_SIZE];
   int numChallenge = find_challenge_by_challenged_client(*sender) ; 
   char invitation[BUF_SIZE];
   int socket_challenger = challenges[numChallenge].challenger->sock  ; 
   int socket_challenged = challenges[numChallenge].challenged->sock  ;
   strcpy(invitation, sender->name) ; 
   strcat(invitation, " a accepté votre challenge.\n") ; 
   
   challenges[numChallenge].state = 1;
   
   reset(challenges[numChallenge].tab, challenges[numChallenge].points, 4) ;
   challenges[total_challenges].turn = rand()%2 ; 
   sender->isChallenged  = 0 ; 
   
   challenges[numChallenge].challenged->isPlaying = 1 ;
   challenges[numChallenge].challenger->isPlaying = 1 ; 
   

   write_client(socket_challenger, invitation);
   print_table_to_char(challenges[numChallenge].tab, challenges[numChallenge].points, challenges[numChallenge].challenged->name, challenges[numChallenge].challenger->name, affichage) ;
   write_client(socket_challenger, affichage); 
   write_client(socket_challenged, affichage);
   
   
  if (challenges[total_challenges].turn == 0 ){ 
      write_client(socket_challenger,"A votre tour" ) ;
      write_client(socket_challenged,"Au tour de votre adversaire" ) ;
      
  } else {
      write_client(socket_challenged,"A votre tour" ) ;
      write_client(socket_challenger,"Au tour de votre adversaire" ) ;
  }
}

/*Permet de gérer le refus d'un challenge*/
void refuse_challenge_request(Client* sender){
   int numChallenge = find_challenge_by_challenged_client(*sender) ;
   char invitation[BUF_SIZE];
   int socket_challenger = challenges[numChallenge].challenger->sock  ; 
   int socket_challenged = challenges[numChallenge].challenged->sock  ;
   snprintf(invitation, BUF_SIZE, "%s a refusé votre challenge.", sender->name);
   challenges[numChallenge].state = 0 ;
   write_client(socket_challenger, invitation);
   sender->isChallenged = 0 ; 
}

/*Permet de gérer les demandes d'ami*/
void handle_friend_request(Client* sender, Client *clients, int actual, const char *buffer){
   
   Client *target = extract_target_by_name(clients, buffer +strlen("/add_friend") +1 , actual);
   if (target == NULL){
      write_client(sender->sock ,"Ce joueur n'existe pas") ; 
      return ; 
   }
   if (isFriend(sender,target)){
      write_client(sender->sock ,"Ce joueur est déjà votre ami") ; 
      return ; 
   }
   if (isPending(sender,target)){
      write_client(sender->sock ,"Cette personne vous a déjà envoyé une requête d'ami") ; 
      return ; 
   }
   if (isPending(target,sender)){
      write_client(sender->sock ,"Vous avez dejà envoyé une requête d'ami à cette personne") ; 
      return ; 
   }
   if(target == sender){
      write_client(sender->sock ,"Vous ne pouvez pas envoyer de requête d'ami à vous même") ;
      return ; 
   }

   if (target->numPendingFriends < MAX_PENDING_FRIEND_REQ) {
      target->pending_friend_request[target->numPendingFriends] = sender; //ajouter la requête au tableau des requêtes d'ami en attente
      target->numPendingFriends++;

      char invitation[BUF_SIZE];
      strcpy(invitation , sender->name) ;
      strcat(invitation , " vous a ajouté en ami.\n/accept [pseudo] ou /decline [pseudo] pour répondre.") ; 
      write_client(target->sock, invitation);
      write_client(sender->sock , "Requête d'ami envoyée") ; 
    } else {
      write_client(sender->sock, "La liste des requêtes d'ami de ce joueur est pleine. Réessayez plus tard.");
    }

}

/*Permet de vérifier si deux Clients sont amis*/
int isFriend(Client* sender, Client* target) {
    for (int i = 0; i < sender->numFriends; ++i) {
        if (sender->friends[i] == target) {
            return 1;
        }
    }
    return 0;
}

/*Permet de vérifier si un Client a envoyé une requête d'ami en attente à un autre Client*/
int isPending(Client* sender, Client* target) {
    for (int i = 0; i < sender->numPendingFriends; ++i) {
        if (sender->pending_friend_request[i] == target) {
            return 1;
        }
    }
    return 0;
}

/*Permet de gérer l'acceptation d'une requête d'ami*/
void accept_friend_request(Client* sender, Client *clients, int actual, const char *buffer){
   Client *target = extract_target_by_name(clients, buffer +strlen("/accept") +1 , actual);
   int foundRequest = 0;
   char message[BUF_SIZE];

   for (int i = 0; i < sender->numPendingFriends; ++i) {
      if (sender->pending_friend_request[i] == target) {
            if (sender->numFriends < MAX_FRIENDS && target->numFriends < MAX_FRIENDS) {
               sender->friends[sender->numFriends] = target; //ajouter le nouvel ami au tableau d'ami
               sender->numFriends++;

               target->friends[target->numFriends] = sender; //ajouter le nouvel ami au tableau d'ami
               target->numFriends++;

               strcpy(message, sender->name) ; 
               strcat(message, " a accepté votre requête d'ami.\n");
               write_client(target->sock, message);

               snprintf(message, BUF_SIZE, "Vous êtes maintenant ami avec %s.", target->name);
               write_client(sender->sock, message);

               for (int j = i; j < sender->numPendingFriends - 1; ++j) {
               sender->pending_friend_request[j] = sender->pending_friend_request[j + 1]; //enlever la requête du tableau des requêtes en cours
               }
               sender->numPendingFriends--;

            } else if(sender->numFriends >= MAX_FRIENDS){
               write_client(sender->sock, "Votre liste d'ami est pleine");
            } else if(target->numFriends >= MAX_FRIENDS){
               snprintf(message, BUF_SIZE, "La liste d'ami de %s est pleine.", target->name);
               write_client(sender->sock, message);
            }

            foundRequest = 1;
      }
   }
   if (!foundRequest) {
      write_client(sender->sock, "Ce joueur ne vous a pas envoyé de requête d'ami.");
   }
}

/*Permet de gérer le refus d'une requête d'ami*/
void decline_friend_request(Client* sender, Client *clients, int actual, const char *buffer){
   Client *target = extract_target_by_name(clients, buffer +strlen("/decline") +1 , actual);
   int foundRequest = 0;
   char message[BUF_SIZE];

   for (int i = 0; i < sender->numPendingFriends; ++i) {
      if (sender->pending_friend_request[i] == target) {
         for (int j = i; j < sender->numPendingFriends - 1; ++j) {
            sender->pending_friend_request[j] = sender->pending_friend_request[j + 1]; //enlever la requête du tableau des requêtes en cours
         }
         sender->numPendingFriends--;

         strcpy(message, sender->name) ; 
         strcat(message, " a refusé votre requête d'ami.\n") ; 
         write_client(target->sock, message);

         foundRequest = 1;
      }
   }
   
   if (!foundRequest) {
      write_client(sender->sock, "Ce joueur ne vous a pas envoyé de requête d'ami.");
   }
}

/*Permet de gérer la suppression d'un ami*/
void unfriend_friend(Client* sender, Client *clients, int actual, const char *buffer){
   Client *target = extract_target_by_name(clients, buffer +strlen("/unfriend") +1 , actual);
   char message[BUF_SIZE];

   if (isFriend(sender,target)) {
      for (int i = 0; i < sender->numFriends; ++i) {
         if (sender->friends[i] == target) {
            for (int j = i; j < sender->numFriends - 1; ++j) {
               sender->friends[j] = sender->friends[j + 1]; //enlever l'ami du tableau des amis
            }
            sender->numFriends--;
         }
      }
      for (int i = 0; i < target->numFriends; ++i) {
         if (target->friends[i] == sender) {
            for (int j = i; j < target->numFriends - 1; ++j) {
               target->friends[j] = target->friends[j + 1]; //enlever l'ami du tableau des amis
            }
            target->numFriends--;
         }
      }

      snprintf(message, BUF_SIZE, "%s a été retiré de vos amis.", target->name);
      write_client(sender->sock, message);

      snprintf(message, BUF_SIZE, "%s vous a retiré de ses amis.", sender->name);
      write_client(target->sock, message);
   } else {
      snprintf(message, BUF_SIZE, "%s ne fait pas partie de vos amis.", target->name);
      write_client(sender->sock, message);
   }

}

/*Permet de trouver le challenge en cours d'un joueur challengé*/
int find_challenge_by_challenged_client(Client challenged){
   for (int i = 0; i < total_challenges ; i++) {
      if (challenges[i].challenged->sock == challenged.sock  && challenges[i].state == -1) {
         return i;
      }
   }
}

/*Permet de gérer le déroulé d'un jeu*/
void handle_game(Client* sender, char* buffer){
   char affichage[BUF_SIZE];
   char fin[BUF_SIZE];
   int numChallenge = find_challenge_by_player(*sender) ;
   int coup  = atoi(buffer) ;
   int socket_challenger  = challenges[numChallenge].challenger->sock; 
   int socket_challenged = challenges[numChallenge].challenged->sock;
   if(socket_challenger == sender->sock){
      if(challenges[numChallenge].turn) {
         if (move_allowed(challenges[numChallenge].tab, &coup, challenges[numChallenge].turn )){
            handle_turn(numChallenge , coup , affichage) ; 
            challenges[numChallenge].turn = 0 ;
            if(!is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points )){
               write_client(socket_challenger,"Au tour de votre adversaire\n" ) ;
               write_client(socket_challenged,"A votre tour\n" ) ;
            }else if(is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 1){
               handle_endgame(numChallenge , socket_challenged , socket_challenger , fin) ;

            }else if(is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 2){
               handle_endgame(numChallenge , socket_challenger , socket_challenged , fin) ;  
            }
         }else{
            write_client(socket_challenger,"Mouvement illégal veuillez réessayer.\n" ) ;
         }
      }else{
         write_client(socket_challenger, "Au tour de votre adversaire\n") ;  
      }
      
   } else if(socket_challenged == sender->sock){
      if(!challenges[numChallenge].turn) {
         if (move_allowed(challenges[numChallenge].tab , &coup, challenges[numChallenge].turn )){
            handle_turn(numChallenge , coup , affichage) ; 
            challenges[numChallenge].turn = 1 ;
            if(!is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points)){
               write_client(socket_challenged,"Au tour de votre adversaire\n" ) ;
               write_client(socket_challenger,"A votre tour\n" ) ;
            }else if(is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 1){
               handle_endgame(numChallenge , socket_challenged , socket_challenger , fin) ; 

            }else if(is_finished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 2){
               handle_endgame(numChallenge , socket_challenger , socket_challenged , fin) ;
            }
         }else{
            write_client(socket_challenged,"Mouvement illégal, veuillez réessayer.\n" ) ;
         }
      }else{
         write_client(socket_challenged,"Au tour de votre adversaire\n" ) ;
      }
   }
}

/*Permet de gérer un tour d'une partie*/
void handle_turn(int numChallenge , int coup , char* affichage){
   turn(challenges[numChallenge].tab, challenges[numChallenge].points, challenges[numChallenge].turn, coup ) ; 
   print_table_to_char(challenges[numChallenge].tab, challenges[numChallenge].points ,challenges[numChallenge].challenged->name, challenges[numChallenge].challenger->name, affichage) ;
   write_client(challenges[numChallenge].challenger->sock, affichage); 
   write_client(challenges[numChallenge].challenged->sock, affichage);
   send_game_to_observers(numChallenge, affichage);  //envoyer le jeu aux observers
}

/*Permet de gérer le forfait d'un joueur*/
void handle_forfait(Client* sender , char* buffer ){
   int numChallenge = find_challenge_by_player(*sender) ;
   int socket_challenger  = challenges[numChallenge].challenger->sock; 
   int socket_challenged = challenges[numChallenge].challenged->sock;
   char fin[BUF_SIZE]; 

   if(socket_challenged == sender->sock){
      write_client(socket_challenger , "Votre adversaire a déclaré forfait \n") ; 
      handle_endgame(numChallenge , socket_challenger , socket_challenged , fin) ;
   }else{
      write_client(socket_challenged , "Votre adversaire a déclaré forfait \n") ; 
      handle_endgame(numChallenge , socket_challenged , socket_challenger , fin) ;
   }
   
}

/*Permet de gérer la fin d'une partie*/
void handle_endgame(int num_chall , int socket_gagnant , int socket_perdant , char* fin){
   write_client(socket_gagnant,"Bravo vous avez gagné ;)\n" ) ;
   write_client(socket_perdant,"Vous avez perdu :(\n" );
   snprintf(fin, BUF_SIZE, "%s a gagné.", challenges[num_chall].challenged->name);
   send_game_to_observers(num_chall, fin); //envoyer le jeu aux observers
   challenges[num_chall].state = 2; 
   challenges[num_chall].challenger->isPlaying = 0 ; 
   challenges[num_chall].challenged->isPlaying = 0 ;
   for (int i = 0; i < challenges[num_chall].nbObservers; i++) { //reinitialiser les observateurs
      challenges[num_chall].observers[i]->isObserving = 0;
      challenges[num_chall].observers[i] = NULL;
   }
   challenges[num_chall].nbObservers = 0;
   char* logo = print_logo_to_char(); 
   write_client(socket_gagnant,logo) ;
   write_client(socket_perdant,logo) ;
   free(logo);
}


/*Permet de trouver le challenge en cours associé à un joueur*/
int find_challenge_by_player(Client player){
   for (int i = 0; i < total_challenges ; i++) {
      if ((challenges[i].challenged->sock == player.sock || challenges[i].challenger->sock == player.sock)&& challenges[i].state ==1 
      && challenges[i].challenged->isPlaying && challenges[i].challenger->isPlaying) {
         return i;
      }
   }
}

/*Permet de vérifier qu'une chaine de caractère est un nombre*/
int est_nombre(const char *chaine) {
    if (chaine == NULL || chaine[0] == '\0') {
        return 0;
    }
    int i = 0;
    while (chaine[i] != '\0') {
        if (chaine[i] < '0' || chaine[i] > '9') {
            return 0; // Si un caractère n'est pas un chiffre, la chaîne n'est pas un nombre
        }
        i++;
    }

    return 1;
}

/*Permet de gérer la discussion entre deux joueurs en partie*/
void handle_discussion1(Client* sender  , char* buffer){
   char affichage[BUF_SIZE] ;  
   int numChallenge = find_challenge_by_player(*sender) ;
   strcpy(affichage , sender->name) ; 
   strcat(affichage, " : ") ;
   char* buffer_start = buffer + strlen("/discuss1") + 1  ;
   strcat(affichage  , buffer_start) ; 
   int socket_challenger  = challenges[numChallenge].challenger->sock  ; 
   int socket_challenged = challenges[numChallenge].challenged->sock  ;
   if (socket_challenger == sender->sock){
      write_client(socket_challenged,affichage ) ; 
   }else{
      write_client(socket_challenger,affichage ) ;    
   }
}

/*Permet de gérer la discussion entre deux joueurs hors partie*/
void handle_discussion(Client* sender  , char* buffer ,Client* clients ,int actual ){
   char affichage[BUF_SIZE] ;
   char name[BUF_SIZE] ; 
   extraire_entre_espaces(buffer , name , sizeof(name)) ; 
   Client *target = extract_target_by_name(clients, name , actual);
   if (target == NULL){
      write_client(sender->sock ,"Le joueur avec lequel vous voulez discuter n'a pas été trouvé") ; 
      return ; 
   }
   char* buffer_start = buffer + strlen("/discuss") + strlen(target->name) + 2  ;
   strcpy(affichage , sender->name) ; 
   strcat(affichage, " : ") ;
   strcat(affichage  , buffer_start) ;
   write_client(target->sock,affichage ) ; 
}

/*Permet de d'extraire la chaine de caractère se trouvant entre deux espaces*/
void extraire_entre_espaces(const char* chaine, char* resultat, size_t tailleResultat) {
   const char* premierEspace = strchr(chaine, ' ');
   if (premierEspace == NULL) {
      resultat[0] = '\0';
      return;
   }
   const char* deuxiemeEspace = strchr(premierEspace + 1, ' ');
   if (deuxiemeEspace == NULL) {
      resultat[0] = '\0';
      return;
   }
   size_t longueur = deuxiemeEspace - (premierEspace + 1);
   if (longueur < tailleResultat) {
      strncpy(resultat, premierEspace + 1, longueur);
      resultat[longueur] = '\0';
   } else {
      strncpy(resultat, premierEspace + 1, tailleResultat - 1);
      resultat[tailleResultat - 1] = '\0';
   }
}

/*Permet de gérer l'observation d'un match*/
void observe_match(Client* sender, const char *buffer) {
   char affichage[BUF_SIZE];
   char* match = buffer + strlen("/spectate ");
   int match_id = atoi(match) - 1;

   if (match_id >= 0 && match_id < total_challenges) {
      if (challenges[match_id].nbObservers < MAX_OBSERVERS) {
         if (can_watch(sender, match_id)) { //si le client peut observer le match
            challenges[match_id].observers[challenges[match_id].nbObservers] = sender;
            challenges[match_id].nbObservers++;
            sender->isObserving = 1; 
            print_table_to_char(challenges[match_id].tab, challenges[match_id].points ,challenges[match_id].challenged->name, challenges[match_id].challenger->name, affichage) ;
            write_client(sender->sock, affichage);
         } else {
            write_client(sender->sock ,"Cette partie est privée"); 
         }
      } else {
         write_client(sender->sock ,"Il y a dejà trop d'observateurs pour ce match"); 
      }
   } else {
      write_client(sender->sock ,"Le match n'a pas été trouvé"); 
   }
}

/*Permet de vérifier qu'un joueur peut bien observer un match*/
int can_watch(Client* sender, int match_id) {
   Client* challenged = challenges[match_id].challenged;
   Client* challenger = challenges[match_id].challenger;
   return(
      (!challenged->private && !challenger->private) ||
      ( challenged->private && !challenger->private && isFriend(sender, challenged)) ||
      (!challenged->private &&  challenger->private && isFriend(sender, challenger)) ||
      ( challenged->private &&  challenger->private && isFriend(sender, challenged) && isFriend(sender, challenger))
   );
}

/*Permet d'envoyer le tableau de jeu aux observeurs de ce jeu*/
void send_game_to_observers(int numChallenge, const char* affichage) {
   for (int i = 0; i < challenges[numChallenge].nbObservers; i++) {
      write_client(challenges[numChallenge].observers[i]->sock, affichage);
   }
}

/*Permet de gérer l'arrêt de l'observation d'un match*/
void stop_observe(Client* sender) {
    for (int i = 0; i < total_challenges; i++) {
        for (int j = 0; j < challenges[i].nbObservers; j++) {
            if (challenges[i].observers[j] == sender) {
                for (int k = j; k < challenges[i].nbObservers - 1; k++) {
                    challenges[i].observers[k] = challenges[i].observers[k + 1]; // Retirer le clients du tableau des observateurs
                }
                challenges[i].nbObservers--;
                sender->isObserving = 0;
                return;
            }
        }
    }
}

/*Permet de gérer le passage en mode privé*/
int switch_private(Client* sender) {
   if (sender->private == 1) {
      write_client(sender->sock ,"Le mode privé est déjà activé"); 
   } else {
      sender->private = 1;
      write_client(sender->sock ,"Le mode privé a été activé"); 
   }
}

/*Permet de gérer le passage en mode public*/
int switch_public(Client* sender) {
   if (sender->private == 0) {
      write_client(sender->sock ,"Le mode public est déjà activé"); 
   } else {
      sender->private = 0;
      write_client(sender->sock ,"Le mode public a été activé"); 
   }
}

/*Permet de trouver le challenge en cours associé à un joueur pour la deconnexion*/
int find_challenge_By_player_for_disconnection(Client player){
   for (int i = 0; i < total_challenges ; i++) {
      if ((challenges[i].challenged->sock == player.sock || challenges[i].challenger->sock == player.sock)&& (challenges[i].state ==1 
      || challenges[i].state == -1)) {
         return i;
      }
   }
   return (- 1)  ; 
}

/*Permet d'afficher un ASCII art du titre du jeu*/
int print_logo() {
   printf("\n");
   printf("   _____                 .__          \n");
   printf("  /  _  \\__  _  _______  |  |   ____  \n");
   printf(" /  /_\\  \\ \\/ \\/ /\\__  \\ |  | _/ __ \\ \n");
   printf("/    |    \\     /  / __ \\|  |_\\  ___/ \n");
   printf("\\____|__  /\\/\\_/  (____  /____/\\___  >\n");
   printf("        \\/             \\/          \\/ \n");
   return 0;
}

/*Permet de convertir l'ASCII art du titre du jeu en char**/
char* print_logo_to_char() {
    char buffer[1000]; 

    snprintf(buffer, sizeof(buffer),
        "\n"
        "   _____                 .__          \n"
        "  /  _  \\__  _  _______  |  |   ____  \n"
        " /  /_\\  \\ \\/ \\/ /\\__  \\ |  | _/ __ \\ \n"
        "/    |    \\     /  / __ \\|  |_\\  ___/ \n"
        "\\____|__  /\\/\\_/  (____  /____/\\___  >\n"
        "        \\/             \\/          \\/ \n");

    char* result = malloc(strlen(buffer) + 1);
    strcpy(result, buffer);

    return result;
}


int main(int argc, char **argv)
{
   init();

   print_logo();

   app();

   end();

   return EXIT_SUCCESS;
}
