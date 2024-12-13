/* main.c */
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "httpRequest.h"

#define PORT "4221"
#define BACKLOG 10
#define MAXDATASIZE 1024

void sendResponse(int fd, char* response){
    int sent;
    if ((sent = send(fd, response, strlen(response), 0)) < 0) {
      fprintf(stderr, "Problem sending response.\n");
      exit(1);
    }
    printf("Size of data sent: %d\n", sent);
    //printf("Data sent: %s\n", response);
}

int handleGetRequest(HTTPRequest request, int sock_fd) {
  FILE* fptr;
  char* filename;

  if (strcmp(request.URI, "/") == 0){ 
    filename = "index.html";
  } else if (strcmp(request.URI, "/favicon.ico") == 0){ // send website icon when requested
    int fd = open("favicon.ico", O_RDONLY);

    if (fd == -1){
      fprintf(stderr, "Could not locate .ico file.\n");
      return -1;
    }

    struct stat st;
    fstat(fd, &st);
    char buf[MAXDATASIZE];
    size_t file_size = st.st_size;

    sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length:%zu\r\n"
      "Content-Type: image/x-icon\r\n\r\n", file_size);
    sendResponse(sock_fd, buf);

    char icon_buf[MAXDATASIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, icon_buf, sizeof(icon_buf))) > 0) {
      send(sock_fd, icon_buf, bytes_read, 0);
    }

    return 0;
  } else {
    char buf[MAXDATASIZE];
    char html[100] = "404: The webpage you are looking for does not exist.\n";
    sprintf(buf, "HTTP/1.1 404\r\nContent-Length:%lu\r\n"
      "Content-Type: text/plain; charset=utf-8\r\n\r\n%s", strlen(html), html);
    sendResponse(sock_fd, buf);
    return -1;
  }

  fptr = fopen(filename, "r");
  char html[600];
  int counter = 0;
  while (fgets(&html[counter], 100, fptr) != NULL){
    counter = strlen(html);
  }
  //printf("HTML: %s\n", html);
  //printf("Strlen: %lu\n", strlen(html));

  char buf[MAXDATASIZE];
  sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length:%lu\r\n"
    "Content-Type: text/html; charset=utf-8\r\n\r\n%s", strlen(html), html);
  sendResponse(sock_fd, buf);

  fclose(fptr);

  return 0;
}

void handleRequest(HTTPRequest request, int sock_fd) {
  switch (request.method){
    case GET:
      handleGetRequest(request, sock_fd);
      break;
    default:
      printf("The HTTP method indicated is not allowed.\n");
  }
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
    int s = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);

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
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
    fprintf(stderr, "Failed to set socket option for reuse of port.\n");
    exit(1);
  }
  
  struct linger so_linger;
  so_linger.l_onoff = 1;
  so_linger.l_linger = 30;

  // SOL_SOCKET indicates that reuse option is in the socket
  if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger) == -1) {
    fprintf(stderr, "Failed to set socket option for linger option.\n");
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

  printf("Listening on port: %s...\n", PORT);

  while (1) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof their_addr;
    int new_fd; // for socket file descriptor returned by 'accept()'
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) < 0) { // just accept
      fprintf(stderr, "Accept failed.\n");
      exit(1);
    }

    // Print client information and note that connection has been accepted
    struct sockaddr_in client_info;
    int peer_name;
    int client_addr_len = sizeof(client_info);

    printf("------------------------------------------------\n");
    if ((peer_name = getpeername(new_fd, (struct sockaddr *) &client_info, (socklen_t*) &client_addr_len)) == -1){
      printf("Connection accepted.\n");
      fprintf(stderr, "Unable to resolve client information.\n");
    } else {
      char client_ip4[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &(client_info.sin_addr), client_ip4, INET_ADDRSTRLEN);
      printf("Connection accepted from client: %s.\n", client_ip4);
    }

    char request_string[MAXDATASIZE];
    int read;
    if ((read = recv(new_fd, &request_string, MAXDATASIZE, 0)) == -1) {
      fprintf(stderr, "Problem receiving request.\n");
      exit(1);
    }
    printf("Size of data received: %d\n", read);
    //printf("Data received: %s\n", request_string);

    HTTPRequest request; 
    int req_status = request_constructor(request_string, &request);
    if (req_status == -1){
      continue;
    }
    printf("Request method: %d, request URI: %s\n", request.method, request.URI);

    handleRequest(request, new_fd);

    close(new_fd);
  }

  return 0;
}
