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
     * Enter/Return
     *
     * Normalize both CR and LF to newline before
     * the Ctrl+A....Ctrl+Z decoder
    */
    if(c=='\r' || c=='\n')
    {
        event.key = '\n';
        event.modifiers = 0;
        return event;
    }
    
    /*
     * CTRL handling
     * Ctrl+A ... Ctrl+Z
     */
    if (
        c >= 1 &&
        c <= 26
    )
    {
        event.key =
            'a' + c - 1;

        event.modifiers =
            MOD_CTRL;

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

    /*
     * Normal key
     */
    event.key = c;

    return event;
}
