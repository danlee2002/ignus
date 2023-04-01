#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#define CTRL_KEY(k) ((k) & 0x1f)
//data
struct termios orig_termios;
//terminal section
//error handling 
void die(const char *s) {
    perror(s);
    exit(1);
}
//disables raw mode and restores default
void disableRawMode() { //restores defaults 
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr");
}
//method to prevent echoing of characters within terminal 
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios; //copies original structure
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
char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

//reads key and exists in event of ctrl + q
void editorProcessorKeypress() {
    char c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}
//refreshes screen
void editorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
}

int main () {
    enableRawMode(); //turns on raw mode
    //program exits as soon as q is pressed, else reads input based on character
    while (1) {
        editorProcessorKeypress();
  }
    return 0; 
}
