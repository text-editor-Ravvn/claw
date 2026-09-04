#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios original;

void disableRawMode(void)
{
    /* Restore cooked terminal behavior before the process leaves. */
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&original);
}

void enableRawMode(void)
{
    /* Configure stdin for immediate, non-echoed character input. */
    if (tcgetattr(STDIN_FILENO, &original) == -1)
        return;

    atexit(disableRawMode);

    struct termios raw = original;

    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}