#include <unistd.h>
#include <sys/select.h>
#include <string.h>
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
            return event;
        }

        char seq[32];

        if (read(STDIN_FILENO, &seq[0], 1) != 1)
        {
            event.key = 27;
            return event;
        }

        /* ALT + key */
        if (seq[0] != '[')
        {
            event.key = seq[0];
            event.modifiers = MOD_ALT;
            return event;
        }

        int len = 1;

        while (len < (int)sizeof(seq) - 1)
        {
            if (read(STDIN_FILENO, &seq[len], 1) != 1)
                break;

            if (
                (seq[len] >= 'A' && seq[len] <= 'D') ||
                seq[len] == '~'
            )
            {
                len++;
                break;
            }

            len++;
        }

        seq[len] = '\0';

        /* Plain arrows */

        if (strcmp(seq, "[A") == 0)
        {
            event.key = ARROW_UP;
            return event;
        }

        if (strcmp(seq, "[B") == 0)
        {
            event.key = ARROW_DOWN;
            return event;
        }

        if (strcmp(seq, "[C") == 0)
        {
            event.key = ARROW_RIGHT;
            return event;
        }

        if (strcmp(seq, "[D") == 0)
        {
            event.key = ARROW_LEFT;
            return event;
        }

        /* Delete */

        if (strcmp(seq, "[3~") == 0)
        {
            event.key = DELETE_KEY;
            return event;
        }

        int modifiers = 0;

        if (strstr(seq, ";2"))
            modifiers |= MOD_SHIFT;

        if (strstr(seq, ";3"))
            modifiers |= MOD_ALT;

        if (strstr(seq, ";5"))
            modifiers |= MOD_CTRL;

        switch (seq[len - 1])
        {
            case 'A':
                event.key = ARROW_UP;
                event.modifiers = modifiers;
                return event;

            case 'B':
                event.key = ARROW_DOWN;
                event.modifiers = modifiers;
                return event;

            case 'C':
                event.key = ARROW_RIGHT;
                event.modifiers = modifiers;
                return event;

            case 'D':
                event.key = ARROW_LEFT;
                event.modifiers = modifiers;
                return event;
        }

        return event;
    }

    return decodeKeyChar(c);
}