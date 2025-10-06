all: server
server: cmd/server.c
	cc -Wall -Wextra -o server cmd/server.c utils/channel.c -lpthread

