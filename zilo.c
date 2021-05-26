#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios; // original terminal state

void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode); // register disableRawMode() to be called on exit

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // turn off XOFF, XON, do not translating CR to NL, ignoring SIGINT
    raw.c_oflag &= ~(OPOST);                                  // turn off output processing feature
    raw.c_cflag |= (CS8);                                     // set character size to 8 bits per byte
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);          // turn off ECHO, CANONICAL feature, ignoring signals
    raw.c_cc[VMIN] = 0;                                       // set the minimum number of bytes of input needed
    raw.c_cc[VTIME] = 1;                                      // set the maximum amount of time to wait in a unit of 100 milisec

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

int main()
{
    enableRawMode();

    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

        if (iscntrl(c))
        {
            // ASCII codes 0–31, 127 are control characters (, so nonprintable).
            // examples:
            //   - Up: 27, 91('['), 65
            //   - Right: 27, 91('['), 67
            //   - Escape: 27
            //   - Enter: 10
            //   - Backspace: 127
            //   - Ctrl + a: 1
            //   - Ctrl + b: 2
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }

        if (c == 'q')
            break;
    }
    return 0;
}