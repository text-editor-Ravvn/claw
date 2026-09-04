#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "input.h"

static int readEscapeByte(char *value)
{
    fd_set readSet;
    struct timeval timeout = {0, 100000};
    int ready;

    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);

    do
    {
        ready = select(STDIN_FILENO + 1, &readSet, NULL, NULL, &timeout);
    } while (ready < 0 && errno == EINTR);

    return ready > 0 && read(STDIN_FILENO, value, 1) == 1;
}

int readKey(void)
{
    /* Read one byte, then recognize the terminal's multi-byte key sequences. */
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (c == '\x1b')
    {
        char seq[2];

        /* A short timeout lets a standalone Escape remain a usable key. */
        if (!readEscapeByte(&seq[0]))
            return '\x1b';
        if (!readEscapeByte(&seq[1]))
            return '\x1b';

        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A':
                    return ARROW_UP;

                case 'B':
                    return ARROW_DOWN;

                case 'C':
                    return ARROW_RIGHT;

                case 'D':
                    return ARROW_LEFT;
            }

            if (seq[1] == '3')
            {
                char terminator;
                if (readEscapeByte(&terminator) && terminator == '~')
                    return DELETE_KEY;
            }
        }

        return '\x1b';
    }

    return c;
}