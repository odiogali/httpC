#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *file;
  file = fopen("message.txt", "r");
  if (file == NULL) {
    perror("Unable to open file.\n");
    return -1;
  }

  char buf[8];
  while (1) {
    size_t read = fread(buf, sizeof(char), 8, file);
    printf("read: ");
    fwrite(buf, sizeof(char), read, stdout);
    printf("\n");

    if (read < 8) {
      break;
    }
  }

  return 0;
}
