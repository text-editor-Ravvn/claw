#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

#include "input.h"

static int readEscapeByte(char *value)
{
    fd_set readSet;
    struct timeval timeout = {0, 100000};

    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);

    int ready;

    do
    {
        ready = select(
            STDIN_FILENO + 1,
            &readSet,
            NULL,
            NULL,
            &timeout
        );
    }
    while (
        ready < 0 &&
        errno == EINTR
    );

    return (
        ready > 0 &&
        read(
            STDIN_FILENO,
            value,
            1
        ) == 1
    );
}

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

    if (
        read(
            STDIN_FILENO,
            &c,
            1
        ) != 1
    )
    {
        return event;
    }

    /*
     * ESC sequences
     * Arrow keys
     * Delete key
     * Alt/Option key
     */
    if (c == '\x1b')
    {
        char seq[2];

        if (!readEscapeByte(&seq[0]))
        {
            event.key = '\x1b';
            return event;
        }

        /*
         * Arrow/Delete sequences
         */
        if (seq[0] == '[')
        {
            if (!readEscapeByte(&seq[1]))
            {
                event.key = '\x1b';
                return event;
            }

            switch (seq[1])
            {
                case 'A':
                    event.key =
                        ARROW_UP;
                    return event;

                case 'B':
                    event.key =
                        ARROW_DOWN;
                    return event;

                case 'C':
                    event.key =
                        ARROW_RIGHT;
                    return event;

                case 'D':
                    event.key =
                        ARROW_LEFT;
                    return event;
            }

            if (seq[1] == '3')
            {
                char terminator;

                if (
                    readEscapeByte(
                        &terminator
                    ) &&
                    terminator == '~'
                )
                {
                    event.key =
                        DELETE_KEY;

                    return event;
                }
            }

            /* Unknown escape sequence, do not map to Alt+[ */
            return event;
        }

        /*
         * Alt / Option key
         */
        event.key =
            seq[0];

        event.modifiers =
            MOD_ALT;

        return event;
    }

    return decodeKeyChar(c);
}