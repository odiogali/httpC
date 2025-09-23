#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_buffer(char *buf, size_t n) {
  printf("read: ");
  fwrite(buf, sizeof(char), n, stdout);
  printf("\n");
}

typedef struct reader {
  char *buf;    // buffer holding sentence; only holds what is read
  int buf_size; // total size of allocated space
} reader_t;

int main() {
  FILE *file = fopen("message.txt", "r");
  if (file == NULL) {
    perror("fopen");
    return -1;
  }

  reader_t reader = {0};
  int done = 0; // whether we are finished reading from file or not

  // First read into temporary buffer, then store in reader's buffer
  int temp_buf_size = 8; // For now, our temp buf size = 8
  char *temp = malloc(sizeof(char) * temp_buf_size);
  int temp_read = 0; // How full is temp buf?
  int free_space = temp_buf_size;

  while (1) {
    free_space = temp_buf_size - temp_read; // space left in buffer
    // fill remaining space in buffer
    size_t read = fread(temp + temp_read, sizeof(char), free_space, file);
    temp_read += read; // note how much more space we have

    // if we ran out of data...
    if (read < free_space)
      done = 1;

    // Find the index of the newline
    int idx = -1;
    for (int i = 0; i < temp_read; i++) {
      if (temp[i] == '\n') {
        idx = i;
        break;
      }
    }

    // We read up to \n if it exists
    size_t toCopy = (idx == -1) ? temp_read : idx;

    char *another_temp = realloc(reader.buf, reader.buf_size + toCopy);
    if (another_temp == NULL) {
      perror("realloc");
      return -1;
    }
    memcpy(another_temp + reader.buf_size, temp, toCopy);

    reader.buf = another_temp;
    reader.buf_size += toCopy;

    size_t skip = (idx == -1) ? toCopy : (toCopy + 1);
    memmove(temp, temp + skip, temp_read - skip);
    temp_read -= skip;

    if (idx != -1) {
      print_buffer(reader.buf, reader.buf_size);
      reader.buf = NULL;
      reader.buf_size = 0;
    }

    if (done)
      break;
  }

  free(reader.buf);
  fclose(file);

  return 0;
}
