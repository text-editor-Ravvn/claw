#include <unistd.h>
#include <sys/select.h>

#include "input.h"

int readKey(void)
{
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (c == '\x1b')
    {
        char seq[2];

        fd_set readSet;
        struct timeval timeout = {0, 100000};

        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);
        if (select(STDIN_FILENO + 1, &readSet, NULL, NULL, &timeout) <= 0)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';

        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);
        timeout.tv_usec = 100000;
        if (select(STDIN_FILENO + 1, &readSet, NULL, NULL, &timeout) <= 0)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
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
                if (read(STDIN_FILENO, &terminator, 1) == 1 && terminator == '~')
                    return DELETE_KEY;
            }
        }

        return '\x1b';
    }

    return c;
}