#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#define IGNUS_VERSION "1.0.0"
#define CTRL_KEY(k) ((k) & 0x1f)
//arrow macros
enum editorKey {
  ARROW_LEFT = 'a',
  ARROW_RIGHT = 'd',
  ARROW_UP = 'w',
  ARROW_DOWN = 's'
};
//global config
struct editorConfig {
  int cx, cy;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};
//appending buffers
struct abuf {
  char *b;
  int len;
};
//intializes default abuf
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

#define ABUF_INIT {NULL, 0}
struct editorConfig E;
//terminal section
//error handling 
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);  
    perror(s);
    exit(1);
}
//disables raw mode and restores default
void disableRawMode() { //restores defaults 
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) die("tcsetattr");
}
//method to prevent echoing of characters within terminal 
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = E.orig_termios; //copies original structure
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //turns off misc flags
    raw.c_iflag &= ~(ICRNL|IXON); //turns off start and output control
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST); //turns off '\n' and '\r\n'
    raw.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG); //turns off echoing/canonical mode/disables signal
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcgetattr");
}
//I/O stuff
//draw tildes

void editorDrawRows(struct abuf *ab) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    if (y == E.screenrows / 3) {
      char welcome[80];
      int welcomelen = snprintf(welcome, sizeof(welcome),
        "Welcome to Ignus -- version %s", IGNUS_VERSION);
      if (welcomelen > E.screencols) welcomelen = E.screencols;
      int padding = (E.screencols - welcomelen) / 2;
      if (padding) {
        abAppend(ab, "~", 1);
        padding--;
      }
      while (padding--) abAppend(ab, " ", 1);
      abAppend(ab, welcome, welcomelen);
    } else {
      abAppend(ab, "~", 1);
    }
    abAppend(ab, "\x1b[K", 3);
    if (y < E.screenrows - 1) {
      abAppend(ab, "\r\n", 2);
    }
  }
}
//waits for keypress 
char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      switch (seq[1]) {
        case 'A': return ARROW_UP;
        case 'B': return ARROW_DOWN;
        case 'C': return ARROW_RIGHT;
        case 'D': return ARROW_LEFT;
      }
      
    }
    return '\x1b';
  } else {
    return c;
  }
}
//gets window size
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) { //checks if valid 
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}






//init
void initEditor() {
  E.cx = 0;
  E.cy = 0;
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}
//code to refresh screen 

void editorRefreshScreen() {
  struct abuf ab = ABUF_INIT;
  abAppend(&ab, "\x1b[?25l", 6);
  abAppend(&ab, "\x1b[H", 3);
  editorDrawRows(&ab);
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));
  abAppend(&ab, "\x1b[?25h", 6);
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}
//handles movement of cursor
void editorMoveCursor(char key) {
  switch (key) {
    case 'a':
      if (E.cx != 0) {
        E.cx--;
      }
      break;
    case 'd':
      if  (E.cx != E.screencols - 1){
        E.cx++;
      }
      
      break;
    case 'w':
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case 's':
      if (E.cy != E.screenrows - 1) {
        E.cy++;
      }
      break;
  }
}
//reads key and exists in event of ctrl + q and handles interrupt
void editorProcessorKeypress() {
    char c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3); 
            exit(0);
            break;
    case 'w':
    case 's':
    case 'a':
    case 'd':
    editorMoveCursor(c);
    break;
    }
}

int main () {
    enableRawMode(); //turns on raw mode
    initEditor();
    //program exits as soon as q is pressed, else reads input based on character
    while (1) {
        editorRefreshScreen(); 
        editorProcessorKeypress();
        write(STDOUT_FILENO, "\x1b[H", 3);
  }
    return 0; 
}