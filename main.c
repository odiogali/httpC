/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]){ // two arguments, first - name of program, second - domain name
  // Create a TCP connection:
  
  // Make sure there is only one argument
  if (argc != 2) {
    fprintf(stderr, "usage: httpC hostname\n");
    return 1;
  }

  int status;
  struct addrinfo hints;
  struct addrinfo *servInfo;
  struct addrinfo *p;

  memset(&hints, 0, sizeof hints); // zero out hints
  hints.ai_family = AF_UNSPEC; // server machine may be IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE; // fill in my IP for me
  
  if ((status = getaddrinfo(argv[1], "8080", &hints, &servInfo)) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 2;
  }

  // We should now have a linked list of addrinfo structs - servInfo
  
  int serverfd;

  // Create a new socket so local machine (server) can communicate with clients
  for (p  = servInfo; p != NULL; p = p->ai_next){
    int s = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);    

    if (s != -1){
      serverfd = s;
      break;
    }
  }

  if (p == NULL) {
    // No valid entries found
    fprintf(stderr, "Failed to create socket.\n");
    exit(3);
  }
  
  // Set sock opt to enable reuse of a port after a server is closed
  int yes = 1;
  if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
    fprintf(stderr, "Failed to set socket option.\n");
    exit(4);
  }

  return 0;
}
