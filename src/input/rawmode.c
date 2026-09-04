#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

static struct termios original;

void disableRawMode(void)
{
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&original);
}

void enableRawMode(void)
{
    tcgetattr(STDIN_FILENO,&original);

    atexit(disableRawMode);

    struct termios raw = original;

    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw);
}