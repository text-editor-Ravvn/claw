#include <unistd.h>

#include "input.h"

int readKey(void)
{
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;

    if (c == '\x1b')
    {
        char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
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
        }

        return '\x1b';
    }

    return c;
}