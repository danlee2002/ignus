#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
struct termios orig_termios;
void disableRawMode() { //restores defaults 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
//method to prevent echoing of characters within terminal 
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios; //copies original structure
    raw.c_iflag &= ~(IXON);
    raw.c_lflag &= ~(ECHO|ICANON); //turns off canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main () {
    enableRawMode(); //turns on raw mode
    char c;
    //program exits as soon as q is pressed, else reads input based on character
    while (read(STDIN_FILENO, &c,1) == 1 && c != 'q') {
        if (iscntrl(c)) {
            printf("%d\n",c);
        } else {
            printf("%d('%c')\n",c,c);
        }
    }
    return 0; 
}
