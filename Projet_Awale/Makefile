CC = gcc
CFLAGS = -Wall -Werror

default: all

all: awale_exec client_exec serveur_exec

awale_exec: awale/awale.c
	$(CC)  -o $@ $<

client_exec: Client/client2.c
	$(CC)  -o $@ $<

serveur_exec: Serveur/server2.c
	$(CC)  -o $@ $<

clean:
	rm -f awale_exec client_exec serveur_exec

.PHONY: all clean
