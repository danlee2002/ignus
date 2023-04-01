#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;

//error handling 
void die(const char *s) {
    perror(s);
    exit(1);
}
//disables raw mode and restores default
void disableRawMode() { //restores defaults 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
//method to prevent echoing of characters within terminal 
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios; //copies original structure
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); //turns off misc flags
    raw.c_iflag &= ~(ICRNL|IXON); //turns off start and output control
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST); //turns off '\n' and '\r\n'
    raw.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG); //turns off echoing/canonical mode/disables signal
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main () {
    enableRawMode(); //turns on raw mode
    //program exits as soon as q is pressed, else reads input based on character
    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
        printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  }
    return 0; 
}
