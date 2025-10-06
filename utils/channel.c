#include "channel.h"
#include <string.h>

void channel_init(channel *ch) {
  pthread_mutex_init(&ch->mutex, NULL);
  pthread_cond_init(&ch->cond, NULL);
  ch->ready = 0;
  ch->closed = 0;
  ch->done = 1;
}

// Function to send a message through the channel
void channel_send(channel *ch, const char *msg) {
  pthread_mutex_lock(&ch->mutex);
  while (!ch->done) {
    pthread_cond_wait(&ch->cond, &ch->mutex);
  }
  strcpy(ch->message, msg);
  ch->ready = 1;
  ch->done = 0;
  pthread_cond_signal(&ch->cond);
  pthread_mutex_unlock(&ch->mutex);
}

// Function to receive a message from the channel
int channel_receive(channel *ch, char *msg) {
  pthread_mutex_lock(&ch->mutex);
  while (!ch->ready && !ch->closed) {
    pthread_cond_wait(&ch->cond, &ch->mutex);
  }
  strcpy(msg, ch->message);
  if (strlen(ch->message) == 0) {
    ch->closed = 1;
    pthread_mutex_unlock(&ch->mutex);
    return 0;
  }
  ch->ready = 0;
  ch->done = 1;
  pthread_cond_signal(&ch->cond);
  pthread_mutex_unlock(&ch->mutex);
  return 1;
}
