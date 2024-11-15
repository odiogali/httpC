/* main.c */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "httpRequest.h"

#define PORT "4221"
#define BACKLOG 10
#define MAXDATASIZE 1024

void handleRequest(HTTPRequest request, char *response) {
  char buf[MAXDATASIZE] = "HTTP/1.1 200 OK\r\n\r\n";
  strcpy(response, buf);
}

int main() {
  struct addrinfo hints; // will contain basic information about our connection
  struct addrinfo *servInfo; // will contain the results of getaddrinfo()
  struct addrinfo *p; // pointer we will use to go through the results of getaddrinfo()

  memset(&hints, 0, sizeof hints); // zero out hints
  hints.ai_family = AF_UNSPEC;     // server machine may be IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets as opposed to datagram
                                   // sockets (used in UDP - connectionless)
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  int status;
  if ((status = getaddrinfo(NULL, PORT, &hints, &servInfo)) !=
      0) { // get this machine's address info
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  // We should now have a linked list of addrinfo structs - servInfo

  int sockfd;

  // Create a new socket so local machine (server) can communicate with clients
  for (p = servInfo; p != NULL; p = p->ai_next) {
    int s = socket(servInfo->ai_family, servInfo->ai_socktype,
                   servInfo->ai_protocol);

    if (s != -1) {
      sockfd = s;
      break;
    }
  }

  if (p == NULL) { // No valid entries found
    fprintf(stderr, "Failed to create socket.\n");
    exit(1);
  }

  // Set sock opt to enable reuse of a port after a server is closed
  int yes = 1;
  // SOL_SOCKET indicates that reuse option is in the socket
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
    fprintf(stderr, "Failed to set socket option.\n");
    exit(1);
  }

  // Bind socket (def by sockfd) to the PORT specified above
  if (bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen) == -1) {
    close(sockfd);
    fprintf(stderr, "Failed to bind socket to the port.\n");
    exit(1);
  }

  freeaddrinfo(servInfo); // Probably calls free() for memory used by servInfo linked list

  // Listen for incoming connections
  if (listen(sockfd, BACKLOG) < 0) {
    fprintf(stderr, "Too many incoming connections, ending session.\n");
    exit(1);
  }

  printf("Listening on port: %s.\n", PORT);

  while (1) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd; // for socket file descriptor returned by 'accept()'
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) <
        0) { // when client tries to connect, accept
      fprintf(stderr, "Accept failed.\n");
      exit(1);
    }

    // For debug purposes, it may be worthwhile to look into printing the IP of
    // the device that has connected with server
    printf("Connection accepted.\n\n");

    char request_string[MAXDATASIZE];
    int read;
    if ((read = recv(new_fd, request_string, MAXDATASIZE - 1, 0)) == -1) {
      fprintf(stderr, "Problem receiving request.\n");
      exit(1);
    }

    HTTPRequest request = request_constructor(request_string);

    char response[MAXDATASIZE];
    handleRequest(request, response);
    if (send(new_fd, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Problem sending response.\n");
      exit(1);
    }

    close(new_fd);
  }

  return 0;
}
