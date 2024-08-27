/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define PORT "8080"
#define BACKLOG 10

int main(){
  struct addrinfo hints; // will contain basic information about our connection
  struct addrinfo *servInfo; // will contain the results of getaddrinfo()
  struct addrinfo *p; // pointer we will use to go through the results of getaddrinfo()

  memset(&hints, 0, sizeof hints); // zero out hints
  hints.ai_family = AF_UNSPEC; // server machine may be IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
  hints.ai_flags = AI_PASSIVE; // fill in my IP for me
  
  int status;
  if ((status = getaddrinfo(NULL, PORT, &hints, &servInfo)) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 2;
  }

  // We should now have a linked list of addrinfo structs - servInfo
  
  int sockfd;

  // Create a new socket so local machine (server) can communicate with clients
  for (p  = servInfo; p != NULL; p = p->ai_next){
    int s = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);    

    if (s != -1){
      sockfd = s;
      break;
    }
  }

  if (p == NULL) { // No valid entries found
    fprintf(stderr, "Failed to create socket.\n");
    exit(3);
  }
  
  // Set sock opt to enable reuse of a port after a server is closed
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1){
    fprintf(stderr, "Failed to set socket option.\n");
    exit(4);
  }

  if (bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen) == -1){
    fprintf(stderr, "Failed to bind socket to a port.\n");
    exit(5);
  }

  freeaddrinfo(servInfo);

  // Listen for incoming connections
  if (listen(sockfd, BACKLOG) < 0){
    fprintf(stderr, "Problem listening.\n");
    exit(6);
  }

  printf("Listening on port: %s.\n", PORT);

  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;
  int new_fd; // for socket file descriptor returned by 'accept()'
  if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) < -1){
    fprintf(stderr, "Accept failed.\n");
    exit(7);
  }

  printf("Connection accepted.");

  return 0;
}
