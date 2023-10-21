CC = gcc
CFLAGS = -lm -Wall -Werror

default: all

all: awale_exec client_exec serveur_exec

awale_exec: awale/awale.c 
	$(CC)  -o $@ $<

client_exec: Client/awale_client.c Client/client2.c 
	$(CC) -o $@ $^ $(CFLAGS)

serveur_exec: Serveur/awale_serveur.c Serveur/server2.c
	$(CC)  -o $@ $^

clean:
	rm -f awale_exec client_exec serveur_exec

.PHONY: all clean
