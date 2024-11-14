#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#define PORT "4221"

int main(int argc, char *argv[]) {
  int status;
  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // IP addresses are per connection (new connection -> new IP) - represents particular machine connected to network at certain time
  status = getaddrinfo("127.0.0.1", PORT, &hints, &res); // get address info of the server machine
  if (status == -1){
    printf("Get addrinfo failed.\n");
    return -1;
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1){
    printf("Failed to initialize socket.\n");
    return -1;
  }

  if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1){ // connect local machine to server using the socket
    close(sockfd);
    fprintf(stderr, "Failed to bind socket to a port.\n");
    exit(1);
  }

  char msg[] = "GET / HTTP/1.1\r\n\r\n";
  int len, bytes_sent;
  len = sizeof(msg);

  bytes_sent = send(sockfd, msg, len, 0);
  
  int bytes_received; 
  char buf[1024];
  if ((bytes_received = recv(sockfd, &buf, sizeof buf, 0))== -1){
    printf("Problem receiving bytes from the server.\n");
    return -1;
  }
  printf("Bytes received: %d, content received: %s", bytes_received, buf);

  return 0;
}
