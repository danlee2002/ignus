#include <string.h>
#include <stdlib.h>
//appending buffers
struct abuf {
  char *b;
  int len;
};

void abAppend(struct abuf *ab, const char *s, int len) {
  //allocates more memoery 
  char *new = realloc(ab -> b, ab->len + len);
  if (new == NULL) return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab -> len += len;
}


void abFree(struct abuf *ab) {
  free(ab->b);
}