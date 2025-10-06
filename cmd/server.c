#include "server.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *get_lines(void *arg) {
  int *result = malloc(sizeof(int));
  channel *ch = (channel *)arg;

  FILE *file = fopen("message.txt", "r");
  if (file == NULL) {
    perror("fopen");
    *result = -1;
    return result;
  }

  char buf[9];
  char *line = calloc(1, 1);

  int read;
  while ((read = fread(buf, sizeof(char), 8, file)) > 0) {
    buf[read] = '\0';

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

  channel_send(ch, "");
  free(line);
  fclose(file);

  *result = 0;
  return result;
}

int main() {
  channel ch;
  channel_init(&ch);

  pthread_t thread;
  pthread_create(&thread, NULL, get_lines, &ch);

  char msg[MAX_MSG_LEN];
  while (channel_receive(&ch, msg)) {
    printf("%s\n", msg);
  }

  pthread_join(thread, NULL);
  return 0;
}
