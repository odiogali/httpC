#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define MAX_MSG_LEN 1024

typedef struct {
  char message[MAX_MSG_LEN];
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int ready; // is there a message to read?
  int done;  // is receiver done reading?
  int closed;
} channel;

void channel_init(channel *ch);
void channel_send(channel *ch, const char *msg);
int channel_receive(channel *ch, char *msg);
void channel_close(channel *ch);
