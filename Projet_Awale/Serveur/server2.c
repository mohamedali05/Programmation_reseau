#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"
#include "awale_serveur.h"

Challenge challenges[MAX_CLIENTS];
int num_challenges = 0;

static void init(void)
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

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

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
                  if (strcmp(buffer, "/list") == 0) {
                     // Handle the "/list" command 
                     handle_list_request(clients[i], clients, actual);
                  } else if ((strstr(buffer, "/challenge") != NULL)) {
                     handle_challenge_request(clients[i], clients, actual, buffer);
                  } else if(strcmp(buffer, "/accept") == 0 && clients[i].isChallenged) {
                     accept_challenge_request(&clients[i], clients , actual);
                  } else if(strcmp(buffer, "/refuse") == 0 && clients[i].isChallenged) {
                     refuse_challenge_request(&clients[i]);
                  }else if (clients[i].isPlaying && estNombre(buffer)){
                        handle_game(&clients[i] , buffer ) ; 
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

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
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

static int init_connection(void)
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

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
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

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}
static void handle_list_request(Client client, Client *clients, int actual) {
    char response[BUF_SIZE];
    response[0] = 0;

   
    printf("handle_list_request \n") ; 

    for (int i = 0; i < actual; i++) {
        strncat(response, clients[i].name, BUF_SIZE - 1);
        if (i != actual -1) {
            strncat(response, "\n", BUF_SIZE - strlen(response) - 1);
        }
    }
   write_client(client.sock, "Liste des joueurs disponibles \n");
   write_client(client.sock, response);
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

static void handle_challenge_request(Client sender, Client *clients, int actual, const char *buffer){
    Client *target = extract_target_by_name(clients, buffer, actual);

    if (target != NULL) {
        // Créez une nouvelle invitation.
        challenges[num_challenges].challenger_sock = sender.sock;
        challenges[num_challenges].challenged_sock = target->sock;
        challenges[num_challenges].accepted = -1;  // -1 indique en attente d'une réponse.

        target->isChallenged = 1;

        // Envoyez l'invitation au client ciblé.
        char invitation[BUF_SIZE];
        snprintf(invitation, BUF_SIZE, "%s vous a défié. /accept ou /refuse pour répondre.", sender.name);
        write_client(target->sock, invitation);
        

        // Incrémente le compteur des invitations.
        num_challenges++;
    }
}

static Client* extract_target_by_name(Client* clients , const char* buffer, int actual){
   for (int i = 0; i < actual; i++) {
         char *name = clients[i].name;
        // Check if the name is present in the buffer
        if (strstr(buffer, name) != NULL) {
            return &(clients[i]); // Name found in the sentence
        }
    }
    
    return NULL; // No name found in the sentence
}

static void accept_challenge_request(Client* sender , Client* Clients ,   int actual){
   // Envoyez l'invitation au client ciblé.
   char affichage[BUF_SIZE];
   int numChallenge = find_challenge_by_challenged_client(*sender) ; 
   char invitation[BUF_SIZE];
   int socket_challenger  = challenges[numChallenge].challenger_sock  ; 
   int socket_challenged = challenges[numChallenge].challenged_sock  ;
   snprintf(invitation, BUF_SIZE, "%s a accepté votre challenge. \n", sender->name);
   challenges[numChallenge].accepted = 1;
   
   reset(challenges[numChallenge].tab  ,  challenges[numChallenge].points , 4 ) ;
   challenges[num_challenges].turn = rand()%2 ; 

   sender->isChallenged  = 0 ; 
   sender->isPlaying = 1 ;
   Clients[find_client_by_socket(socket_challenger,Clients , actual)].isPlaying = 1 ; 


   write_client(socket_challenger, invitation);
   printTableToChar(challenges[numChallenge].tab  ,challenges[numChallenge].points , affichage) ;
   write_client(socket_challenger,affichage ) ; 
   write_client(socket_challenged, affichage) ;
   snprintf(affichage,BUF_SIZE, "Le tour est au joueur %d",  challenges[num_challenges].turn);
  
   write_client(socket_challenger,affichage ) ; 
   write_client(socket_challenged, affichage) ; 
}

static void refuse_challenge_request(Client* sender){
   // Envoyez l'invitation au client ciblé.

   int numChallenge = find_challenge_by_challenged_client(*sender) ;
   char invitation[BUF_SIZE];
   int socket_challenger  = challenges[numChallenge].challenger_sock  ; 
   int socket_challenged = challenges[numChallenge].challenged_sock  ;
   snprintf(invitation, BUF_SIZE, "%s a refusé votre challenge.", sender->name);
   challenges[numChallenge].accepted = 0 ;
   write_client(socket_challenger , invitation);
   sender->isChallenged = 0 ; 
}

static void handle_game(Client* sender  , char* buffer ){
   char affichage[BUF_SIZE];
   int numChallenge = find_challenge_by_player(*sender) ;
   int coup  = atoi(buffer) ;
   int socket_challenger  = challenges[numChallenge].challenger_sock  ; 
   int socket_challenged = challenges[numChallenge].challenged_sock  ;
   if(socket_challenger == sender->sock && challenges[numChallenge].turn
   && moveAllowed(challenges[numChallenge].tab , &coup, challenges[numChallenge].turn )){
      //the challenger sended the request and it's his turn 
      turn(challenges[numChallenge].tab , challenges[numChallenge].points ,  challenges[numChallenge].turn , coup ) ; 
      printTableToChar(challenges[numChallenge].tab  ,challenges[numChallenge].points , affichage) ;
      challenges[numChallenge].turn = 0 ; 
      write_client(socket_challenger,affichage ) ; 
      write_client(socket_challenged, affichage) ;
      
   }else if(socket_challenged == sender->sock && !challenges[numChallenge].turn
   && moveAllowed(challenges[numChallenge].tab , &coup, challenges[numChallenge].turn )){
      //the challenge sended the request and it's his turn 
      turn(challenges[numChallenge].tab , challenges[numChallenge].points ,  challenges[numChallenge].turn , coup ) ; 
      printTableToChar(challenges[numChallenge].tab  ,challenges[numChallenge].points , affichage) ;
      challenges[numChallenge].turn = 1 ; 
      write_client(socket_challenger,affichage ) ; 
      write_client(socket_challenged, affichage) ;

   }
   
}


static int find_challenge_by_challenged_client(Client challenged){
   for (int i = 0; i < num_challenges ; i++) {
      if (challenges[i].challenged_sock == challenged.sock) {
         return i;
      }
   }
}

static int find_challenge_by_player(Client player){
   for (int i = 0; i < num_challenges ; i++) {
      if (challenges[i].challenged_sock == player.sock || challenges[i].challenger_sock == player.sock) {
         return i;
      }
   }
}

static int find_client_by_socket(int sock_client, Client* Clients , int actual ){
   for (int i = 0 ; i<actual ; i++){
      if(Clients[i].sock == sock_client){
         return i ; 
      }
   }
}










int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
