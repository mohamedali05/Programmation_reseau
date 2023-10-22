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

#define BUF_SIZE    1024

#include "client2.h"
typedef struct {
    Client* challenger ; 
    Client* challenged ; 

    
    int accepted; // Indique si le défi a été fini(2) accepté (1) ou refusé (0).
    int tab[12] ; //le tableau du jeu
    int points[2] ; 
    int turn; //0->challenged_sock /1->challenger_sock
} Challenge;


static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);
static void handle_list_request (Client client, Client *clients, int actual); 
static void view_list_matches(Client* sender) ; 
static void handle_challenge_request(Client* sender, Client *clients, int actual, const char *buffer) ;
static void accept_challenge_request(Client* sender , Client* Clients , int actual);
static void refuse_challenge_request(Client* sender);
static void handle_game(Client* sender  , char* buffer ) ; 
static int find_challenge_by_challenged_client(Client challenged);
static void define_bio(Client* sender ,char*bio) ; 
static void handle_discussion1(Client* sender  , char* buffer) ;
// c'est un raccourci pour n'envoyer un message que à notre adversaire  
static void handle_discussion(Client* sender  , char* buffer ,Client* clients ,int actual) ;
int estNombre(const char *chaine) ; 
static  Client* extract_target_by_name(Client* clients , const char* name, int actual) ; 
static int find_challenge_by_player(Client player) ; 
//static int find_client_by_socket(int sock_client, Client* Clients , int actual ) ; 
 
   

#endif /* guard */
