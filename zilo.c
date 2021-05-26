#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios; // original terminal state

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode); // register disableRawMode() to be called on exit

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(IXON);                          // turn off XOFF, XON
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // turn off ECHO, CANONICAL feature, ignoring signals

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
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
            printf("%d\n", c);
        }
        else
        {
            printf("%d ('%c')\n", c, c);
        }
    }
    return 0;
}