#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"
#include "awale_serveur.h"

Challenge challenges[MAX_CLIENTS];
int num_challenges = 0;

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
    {"/accept", "Accepter un challenge en attente"},
    {"/refuse", "Refuser un challenge en attente"},

    {"/define_bio [your_bio]", "Définir sa biographie"},
    {"/view_bio [pseudo]", "Voir la biographie de [pseudo]"},

    {"/view_matches", "Voir les matchs en cours"},

    {"/list_players", "Voir la liste des joueurs en ligne"},
    {"/list_command", "Revoir la liste des commandes disponibles"},
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
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {
                  if (strcmp(buffer,"/list_command") == 0 ){
                     write_client(clients[i].sock, infos_commands);
                  } else if (strcmp(buffer, "/list_players") == 0) {
                     handle_list_request(&clients[i], clients, actual);
                  } else if ((strstr(buffer, "/define_bio") != NULL) ){
                     define_bio(&clients[i] , buffer) ; 
                  }else if(strstr(buffer, "/view_bio")!= NULL){
                     view_bio(&clients[i] ,clients , actual,   buffer) ;
                  }else if(strcmp(buffer, "/view_matches") == 0){
                     view_list_matches(&clients[i]) ; 
                  } else if ((strstr(buffer, "/challenge") != NULL)) {
                     handle_challenge_request(&clients[i], clients, actual, buffer);
                  } else if(strcmp(buffer, "/accept") == 0 && clients[i].isChallenged) {
                     accept_challenge_request(&clients[i], clients , actual);
                  } else if(strcmp(buffer, "/refuse") == 0 && clients[i].isChallenged) {
                     refuse_challenge_request(&clients[i]);
                  } else if (clients[i].isPlaying && estNombre(buffer)){
                     handle_game(&clients[i], buffer) ; 
                  } else if(clients[i].isPlaying && (strstr(buffer, "/discuss1") != NULL)){
                     handle_discussion1(&clients[i], buffer) ; 
                  }else if((strstr(buffer, "/discuss1") == NULL) && (strstr(buffer, "/discuss") != NULL)){
                     handle_discussion(&clients[i], buffer, clients, actual) ; 
                  } else {
                     write_client(clients[i].sock, "La commande est incorrecte");
                  }
                  //send_message_to_all_clients(clients, client, actual, buffer, 0);
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

void define_bio(Client* sender ,char*buffer){
   char* bio_start = buffer + strlen("/define_bio ") ;
   strcpy(sender->bio, bio_start) ; 
   write_client(sender->sock ,"Votre bio a bien été enregistré") ; 
}

void view_bio(Client* sender , Client* clients ,int actual , char* buffer){
   Client *target = extract_target_by_name(clients, buffer+strlen("/view_bio") +1, actual);
   if (target == NULL){
      write_client(sender->sock ,"Ce joueur n'existe pas") ; 
      return ; 
   }
   write_client(sender->sock,target->bio) ; 
}

void view_list_matches(Client* sender){
   char affichage[BUF_SIZE] = "";
   int foundMatch = 0;
   for (int i = 0 ; i<num_challenges ; i++){
      if (challenges[i].state == 1) {
         foundMatch = 1;
         char temp[BUF_SIZE] ; 
         snprintf(temp, BUF_SIZE, "%d : %s vs %s", i + 1, challenges[i].challenged->name, challenges[i].challenger->name);
         strncat(affichage, temp, BUF_SIZE - strlen(affichage) - 1);
      }
   }
   if (!foundMatch){
      write_client(sender->sock, "Il n'y a pas de match en cours");
   } else {
      write_client(sender->sock, affichage);
   }
}

void handle_challenge_request(Client* sender, Client *clients, int actual, const char *buffer){
   Client *target = extract_target_by_name(clients, buffer +strlen("/challenge") +1 , actual);
   if (target == NULL){
      write_client(sender->sock ,"Ce joueur n'existe pas") ; 
      return ; 
   }
   if (target->isPlaying){
      write_client(sender->sock ,"Ce joueur est déjà en train de jouer ") ; 
      return ; 
   }
   
   // Créez une nouvelle invitation.
   challenges[num_challenges].challenger = sender ; 
   challenges[num_challenges].challenged = target ;    
   challenges[num_challenges].state = -1;  // -1 indique en attente d'une réponse.
   target->isChallenged = 1;

   // Envoyez l'invitation au client ciblé.
   char invitation[BUF_SIZE];
   strcpy(invitation , sender->name) ; 
   strcat(invitation , " vous a défié.\n/accept ou /refuse pour répondre.") ; 
   write_client(challenges[num_challenges].challenged->sock, invitation);
   write_client(challenges[num_challenges].challenger->sock , "Challenge envoyé") ; 
   // Incrémente le compteur des invitations.
   num_challenges++;
}

Client* extract_target_by_name(Client* clients , const char* buffer, int actual){ //Get the client with its pseudo
   for (int i = 0; i < actual; i++) {
         char *name = clients[i].name;
        // Check if the name is present in the buffer
        if (strcmp(buffer, name) == 0) {
            return &(clients[i]); // Name found in the sentence
        }
    }
    return NULL; // No name found in the sentence
}

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
   challenges[num_challenges].turn = rand()%2 ; 
   printf("%d \n", challenges[num_challenges].turn) ; 
   sender->isChallenged  = 0 ; 
   
   challenges[numChallenge].challenged->isPlaying = 1 ;
   challenges[numChallenge].challenger->isPlaying = 1 ; 
   

   write_client(socket_challenger, invitation);
   printTableToChar(challenges[numChallenge].tab, challenges[numChallenge].points, challenges[numChallenge].challenged->name, challenges[numChallenge].challenger->name, affichage) ;
   write_client(socket_challenger, affichage) ; 
   write_client(socket_challenged, affichage) ;
   
   
  if (challenges[num_challenges].turn == 0 ){ 
      write_client(socket_challenger,"A votre tour" ) ;
      write_client(socket_challenged,"Au tour de votre adversaire" ) ;
      
  } else {
      write_client(socket_challenged,"A votre tour" ) ;
      write_client(socket_challenger,"Au tour de votre adversaire" ) ;
  }
}

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

int find_challenge_by_challenged_client(Client challenged){ //return the index of the pending challenge of challenged player
   for (int i = 0; i < num_challenges ; i++) {
      if (challenges[i].challenged->sock == challenged.sock  && challenges[i].state == -1) {
         return i;
      }
   }
}

 void handle_game(Client* sender, char* buffer){
   char affichage[BUF_SIZE];
   int numChallenge = find_challenge_by_player(*sender) ;
   int coup  = atoi(buffer) ;
   int socket_challenger  = challenges[numChallenge].challenger->sock  ; 
   int socket_challenged = challenges[numChallenge].challenged->sock  ;
   if(socket_challenger == sender->sock){
      if(challenges[numChallenge].turn) {
         //the challenger sended the request and it's his turn
         if (moveAllowed(challenges[numChallenge].tab, &coup, challenges[numChallenge].turn )){
            turn(challenges[numChallenge].tab, challenges[numChallenge].points,  challenges[numChallenge].turn, coup ) ; 
            printTableToChar(challenges[numChallenge].tab, challenges[numChallenge].points, challenges[numChallenge].challenged->name, challenges[numChallenge].challenger->name , affichage) ;
            write_client(socket_challenger, affichage) ; 
            write_client(socket_challenged, affichage) ;
            challenges[numChallenge].turn = 0 ;
            if(!isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points )){
               write_client(socket_challenger,"Au tour de votre adversaire\n" ) ;
               write_client(socket_challenged,"A votre tour\n" ) ;
            }else if(isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 1){
               write_client(socket_challenged,"Bravo vous avez gagné ;)\n" ) ;
               write_client(socket_challenger,"vous avez perdu :(\n" ) ;
               challenges[numChallenge].state = 2; 
               challenges[numChallenge].challenger->isPlaying = 0 ; 
               challenges[numChallenge].challenger->isPlaying = 0 ;
            }else if(isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 2){
               write_client(socket_challenger,"Bravo vous avez gagné ;)\n" ) ;
               write_client(socket_challenged,"vous avez perdu :(\n" ) ;
               challenges[numChallenge].state = 2; 
               challenges[numChallenge].challenger->isPlaying = 0 ; 
               challenges[numChallenge].challenger->isPlaying = 0 ;
            }
         }else{
            write_client(socket_challenger,"Mouvement illégal veuillez réessayer.\n" ) ;
         }
      }else{
         write_client(socket_challenger, "Au tour de votre adversaire\n") ;  
      }
      
   } else if(socket_challenged == sender->sock){
      if(!challenges[numChallenge].turn) {
         if (moveAllowed(challenges[numChallenge].tab , &coup, challenges[numChallenge].turn )){
            turn(challenges[numChallenge].tab, challenges[numChallenge].points, challenges[numChallenge].turn, coup ) ; 
            printTableToChar(challenges[numChallenge].tab, challenges[numChallenge].points ,challenges[numChallenge].challenged->name, challenges[numChallenge].challenger->name, affichage) ;
            write_client(socket_challenger, affichage) ; 
            write_client(socket_challenged, affichage) ;
            challenges[numChallenge].turn = 1 ;
            if(!isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points)){
               write_client(socket_challenged,"Au tour de votre adversaire\n" ) ;
               write_client(socket_challenger,"A votre tour\n" ) ;
            }else if(isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 1){
               write_client(socket_challenged,"Bravo, vous avez gagné ;)\n" ) ;
               write_client(socket_challenger,"Vous avez perdu :(\n" ) ;
               challenges[numChallenge].state = 2; 
               challenges[numChallenge].challenger->isPlaying = 0 ; 
               challenges[numChallenge].challenger->isPlaying = 0 ; 
            }else if(isFinished(challenges[numChallenge].tab ,challenges[numChallenge].points) == 2){
               write_client(socket_challenger,"Bravo, vous avez gagné ;)\n" ) ;
               write_client(socket_challenged,"Vous avez perdu :(\n" ) ;
               challenges[numChallenge].state = 2; 
               challenges[numChallenge].challenger->isPlaying = 0 ; 
               challenges[numChallenge].challenger->isPlaying = 0 ;
            }
         }else{
            write_client(socket_challenged,"Mouvement illégal, veuillez réessayer.\n" ) ;
         }
      }else{
         write_client(socket_challenged,"Au tour de votre adversaire\n" ) ;
      }
   }
}

int find_challenge_by_player(Client player){
   for (int i = 0; i < num_challenges ; i++) {
      if ((challenges[i].challenged->sock == player.sock || challenges[i].challenger->sock == player.sock)&& challenges[i].state ==1 
      && challenges[i].challenged->isPlaying && challenges[i].challenger->isPlaying) {
         return i;
      }
   }
}

int estNombre(const char *chaine) {
    // Vérifier si la chaîne est vide ou nulle
    if (chaine == NULL || chaine[0] == '\0') {
        return 0;
    }
    // Indice pour parcourir la chaîne
    int i = 0;
    // Parcourir le reste de la chaîne
    while (chaine[i] != '\0') {
        if (chaine[i] < '0' || chaine[i] > '9') {
            return 0; // Si un caractère n'est pas un chiffre, la chaîne n'est pas un nombre
        }
        i++;
    }

    return 1;
}

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

 void handle_discussion(Client* sender  , char* buffer ,Client* clients ,int actual ){
   char affichage[BUF_SIZE] ;
   Client *target = extract_target_by_name(clients, buffer +strlen("/discuss") +1 , actual);
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
 void observe_match(Client* sender , char* buffer){
   
}
/*
 int find_client_by_socket(int sock_client, Client* Clients , int actual ){
   for (int i = 0 ; i<actual ; i++){
      if(Clients[i].sock == sock_client){
         return i ; 
      }
   }
}
*/







int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
