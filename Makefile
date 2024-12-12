all: server client

server: server.c
	cc -Wall -Wextra -o server server.c httpRequest.h httpRequest.c 

client: client.c
	cc -Wall -Wextra -o client client.c
