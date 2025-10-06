#include "server.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "42069" // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold

typedef struct {
  channel *ch;
  int fd;
} thread_param;

void *get_lines(void *arg) {
  int *result = malloc(sizeof(int));

  thread_param *tp = (thread_param *)arg;
  channel *ch = tp->ch;
  int fd = tp->fd;

  char buf[9];
  char *line = calloc(1, 1);

  int r;
  while ((r = read(fd, buf, sizeof(char) * 8)) > 0) {
    buf[r] = '\0';

    char *start = buf;
    for (int i = 0; i < 8; i++) {
      if (buf[i] == '\n') {
        buf[i] = '\0';

        char *temp = malloc(strlen(buf) + strlen(line) + 1);
        strcpy(temp, line);
        strcat(temp, buf);

        channel_send(ch, temp);
        free(line);
        free(temp);
        line = calloc(1, 1);

        start = &buf[i + 1];
      }
    }

    char *temp = malloc(strlen(start) + strlen(line) + 1);
    strcpy(temp, line);
    strcat(temp, start);
    free(line);
    line = temp;
  }

  if (strlen(line) > 0) {
    channel_send(ch, line);
  }

  // Indicate the channel that we are done
  channel_send(ch, "");
  free(line);

  *result = 0;
  return result;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Initializes server, returning a socket file descriptor
int init_server() {
  // listen on sock_fd, new connection on new_fd
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int yes = 1;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  return sockfd;
}

int main() {
  struct sockaddr_storage their_addr; // connector's address info
  socklen_t sin_size;
  int sock_fd, new_fd;
  char s[INET6_ADDRSTRLEN];
  sock_fd = init_server();

  if (listen(sock_fd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  while (1) {
    channel ch;
    channel_init(&ch);

    sin_size = sizeof their_addr;
    new_fd = accept(sock_fd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);

    printf("server: got connection from %s\n", s);

    thread_param tp = {
        .ch = &ch,
        .fd = new_fd,
    };
    pthread_t thread;
    pthread_create(&thread, NULL, get_lines, &tp);

    char msg[MAX_MSG_LEN];
    while (channel_receive(&ch, msg)) {
      printf("%s\n", msg);
    }
    pthread_join(thread, NULL);
    close(new_fd);
  }

  return 0;
}
