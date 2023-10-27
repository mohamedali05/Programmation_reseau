#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define MAX_FRIENDS     20
#define MAX_PENDING_FRIEND_REQ     50
#define MAX_OBSERVERS     1


#define BUF_SIZE    1024

#include "client2.h"
typedef struct {
    Client* challenger; 
    Client* challenged;  
    int state; // Indique si le défi a été fini(2) accepté (1) refusé (0) ou en attente (-1).
    int tab[12]; //le tableau du jeu
    int points[2]; 
    int turn; //0->challenged_sock /1->challenger_sock
    Client* observers[MAX_OBSERVERS]; //tableau des observers du challenge
    int nbObservers;
    int Client_disconnected; 
} Challenge;

typedef struct {
    char command[50] ;
    char description[200] ; 
} Commands; // structure pour décrire les différentes commandes du jeu
 


void init(void);
void end(void);
void app(void);
int init_connection(void);
void end_connection(int sock);
int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);
void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
void remove_client(Client *clients, int to_remove, int *actual);
void clear_clients(Client *clients, int actual);

void handle_list_request (Client* sender, Client *clients, int actual);
void handle_friend_list(Client* sender);
void handle_pending_friend_list(Client* sender);
void view_list_matches(Client* sender);
void define_bio(Client* sender ,char*bio); 
void view_bio(Client* sender , Client* clients ,int actual , char* buffer);

void handle_friend_request(Client* sender, Client *clients, int actual, const char *buffer);
void accept_friend_request(Client* sender, Client *clients, int actual, const char *buffer);
void decline_friend_request(Client* sender, Client *clients, int actual, const char *buffer);

void handle_challenge_request(Client* sender, Client *clients, int actual, const char *buffer);
void accept_challenge_request(Client* sender , Client* Clients , int actual);
void refuse_challenge_request(Client* sender);
void handle_game(Client* sender  , char* buffer );  
void handle_forfait(Client* sender , char* buffer );  
void handle_endgame(int num_chall , int socket_gagnant , int socket_perdant , char* fin) ; 
void handle_discussion1(Client* sender  , char* buffer); 
void handle_discussion(Client* sender  , char* buffer ,Client* clients ,int actual);
void observe_match(Client* sender, const char *buffer);
void send_game_to_observers(int numChallenge, const char* affichage);
void stop_observe(Client* sender);

int switch_public(Client* sender);
int switch_private(Client* sender);
int can_watch(Client* sender, int match_id);
int est_nombre(const char *chaine); 
Client* extract_target_by_name(Client* clients , const char* name, int actual);
int find_challenge_by_challenged_client(Client challenged);
int find_challenge_by_player(Client player); 
int find_challenge_By_player_for_disconnection(Client player) ; 
void extraire_entre_espaces(const char* chaine, char* resultat, size_t tailleResultat) ; 

int print_logo() ;
char* print_logo_to_char();
 
   

#endif /* guard */
