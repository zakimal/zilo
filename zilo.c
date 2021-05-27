/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k)&0x1f) // bitwise-AND with 00011111

/*** data ***/

struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios orig_termios; // original terminal state
};

struct editorConfig E;

/*** terminal ***/

void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode); // register disableRawMode() to be called on exit

    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // turn off XOFF, XON, do not translating CR to NL, ignoring SIGINT
    raw.c_oflag &= ~(OPOST);                                  // turn off output processing feature
    raw.c_cflag |= (CS8);                                     // set character size to 8 bits per byte
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);          // turn off ECHO, CANONICAL feature, ignoring signals
    raw.c_cc[VMIN] = 0;                                       // set the minimum number of bytes of input needed
    raw.c_cc[VTIME] = 1;                                      // set the maximum amount of time to wait in a unit of 100 milisec

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

char editorReadKey()
{
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    // TIOCGWINSZ stands for Terminal Input/Output Control Get WINdow SiZe
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1;
    }
    else
    {
        // pass the values back by references (common approach to having functions return multiple values in C)
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** output ***/

void editorDrawRows()
{
    int y;
    for (y = 0; y < E.screenrows; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen()
{
    // escape sequence
    // - \x1b = escape
    // - <esc>[1J   = clear the screen up to where the cursor is
    // - <esc>[0J   = clear the screen from the cursor up to the end of the screen
    // - <esc>[1;1H = set the cursor at the left-top of the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch (c)
    {
    case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
    }
}

/*** init ***/

void initEditor()
{
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");
}

int main()
{
    enableRawMode();

    initEditor();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}