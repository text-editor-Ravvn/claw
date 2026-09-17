#include <unistd.h>
#include <sys/select.h>

#include "input.h"


KeyEvent decodeKeyChar(char c)
{
    KeyEvent event;
    event.key = 0;
    event.modifiers = 0;

    /*
     * Enter / Newline keys:
     * In standard terminals, Enter transmits '\r' (ASCII 13) or '\n' (ASCII 10).
     * These must not be treated as Ctrl+M or Ctrl+J.
     */
    if (c == '\r' || c == '\n')
    {
        event.key = '\r';
        event.modifiers = 0;
        return event;
    }

    /*
     * Tab key:
     * In standard terminals, Tab transmits '\t' (ASCII 9).
     * This must not be treated as Ctrl+I.
     */
    if (c == '\t')
    {
        event.key = '\t';
        event.modifiers = 0;
        return event;
    }

    /*
     * Backspace key:
     * Terminals send 127 (DEL) or 8 (BS).
     * Standardize to 127 with no modifiers.
     */
    if (c == 127 || c == 8)
    {
        event.key = 127;
        event.modifiers = 0;
        return event;
    }

    /*
     * CTRL handling:
     * Ctrl+A ... Ctrl+Z (excluding Enter, Tab, Backspace handled above)
     */
    if (c >= 1 && c <= 26)
    {
        event.key = 'a' + c - 1;
        event.modifiers = MOD_CTRL;
        return event;
    }

    /*
     * Normal printable character
     */
    event.key = (unsigned char)c;
    return event;
}

KeyEvent readKey(void)
{
    KeyEvent event;

    event.key = 0;
    event.modifiers = 0;

    char c;

    if (read(STDIN_FILENO, &c, 1) != 1)
        return event;

    if (c == '\x1b')
    {
        fd_set set;
        struct timeval tv;

        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);

        tv.tv_sec = 0;
        tv.tv_usec = 30000;

        int ready =
            select(
                STDIN_FILENO + 1,
                &set,
                NULL,
                NULL,
                &tv
            );

        if (ready == 0)
        {
            event.key = 27;
            event.modifiers = 0;
            return event;
        }

        char seq[2];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
        {
            event.key = 27;
            return event;
        }

        if (seq[0] == '[')
        {
            if (read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                event.key = 27;
                return event;
            }

            switch (seq[1])
            {
                case 'A':
                    event.key = ARROW_UP;
                    return event;

                case 'B':
                    event.key = ARROW_DOWN;
                    return event;

                case 'C':
                    event.key = ARROW_RIGHT;
                    return event;

                case 'D':
                    event.key = ARROW_LEFT;
                    return event;
            }

            if (seq[1] == '3')
            {
                char term;

                if (
                    read(STDIN_FILENO,
                         &term,
                         1) == 1 &&
                    term == '~'
                )
                {
                    event.key = DELETE_KEY;
                    return event;
                }
            }

            return event;
        }

        event.key = seq[0];
        event.modifiers = MOD_ALT;

        return event;
    }

    return decodeKeyChar(c);
}