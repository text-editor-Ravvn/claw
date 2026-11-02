#include <stdio.h>
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

    if (c == 0)
    {
        event.key = '`';
        event.modifiers = MOD_CTRL;
        return event;
    }

    if (c == 28)
    {
        event.key = '\\';
        event.modifiers = MOD_CTRL;
        return event;
    }

    if (c == 31)
    {
        event.key = '-';
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
                (seq[len] >= 'A' && seq[len] <= 'Z') ||
                (seq[len] >= 'a' && seq[len] <= 'z') ||
                seq[len] == '~'
            )
            {
                len++;
                break;
            }

            len++;
        }

        seq[len] = '\0';

        /* CSI 27 ; <mod> ; <code> ~ (modifyOtherKeys) */
        int code = 0, mod = 0;
        if (sscanf(seq, "[27;%d;%d~", &mod, &code) == 2)
        {
            int m = 0;
            if (mod == 2) m = MOD_SHIFT;
            else if (mod == 3) m = MOD_ALT;
            else if (mod == 5) m = MOD_CTRL;
            else if (mod == 6) m = MOD_CTRL | MOD_SHIFT;
            event.key = code;
            event.modifiers = m;
            return event;
        }

        /* CSI <code> ; <mod> u (kitty / CSI u) */
        if (sscanf(seq, "[%d;%du", &code, &mod) == 2)
        {
            int m = 0;
            if (mod == 2) m = MOD_SHIFT;
            else if (mod == 3) m = MOD_ALT;
            else if (mod == 5) m = MOD_CTRL;
            else if (mod == 6) m = MOD_CTRL | MOD_SHIFT;
            event.key = code;
            event.modifiers = m;
            return event;
        }

        /* CSI 1 ; <mod> I (Tab with modifiers) */
        if (sscanf(seq, "[1;%dI", &mod) == 1)
        {
            int m = (mod == 5) ? MOD_CTRL : (mod == 6 ? (MOD_CTRL | MOD_SHIFT) : 0);
            event.key = '\t';
            event.modifiers = m;
            return event;
        }

        /* CSI 1 ; <mod> Z (Backtab with modifiers) */
        if (sscanf(seq, "[1;%dZ", &mod) == 1)
        {
            int m = (mod == 5) ? MOD_CTRL : (mod == 6 ? (MOD_CTRL | MOD_SHIFT) : MOD_SHIFT);
            event.key = '\t';
            event.modifiers = m;
            return event;
        }

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

        /* Page Up / Page Down */
        if (strcmp(seq, "[5~") == 0)
        {
            event.key = PAGE_UP;
            return event;
        }

        if (strcmp(seq, "[6~") == 0)
        {
            event.key = PAGE_DOWN;
            return event;
        }

        /* Delete */

        if (strcmp(seq, "[3~") == 0)
        {
            event.key = DELETE_KEY;
            return event;
        }

        /* Shift-Tab (backtab) */

        if (strcmp(seq, "[Z") == 0)
        {
            event.key = BACKTAB;
            return event;
        }

        /* Home */

        if (strcmp(seq, "[H") == 0)
        {
            event.key = HOME_KEY;
            return event;
        }

        /* End */

        if (strcmp(seq, "[F") == 0)
        {
            event.key = END_KEY;
            return event;
        }

        /* Function Keys F1 - F12 */
        int fNum = 0, fMod = 0;
        if (sscanf(seq, "[%d;%d~", &fNum, &fMod) == 2 || sscanf(seq, "[%d~", &fNum) >= 1)
        {
            int m = 0;
            if (fMod == 2) m = MOD_SHIFT;
            else if (fMod == 3) m = MOD_ALT;
            else if (fMod == 5) m = MOD_CTRL;
            else if (fMod == 6) m = MOD_CTRL | MOD_SHIFT;

            int targetKey = 0;
            if (fNum == 11) targetKey = KEY_F1;
            else if (fNum == 12) targetKey = KEY_F2;
            else if (fNum == 13) targetKey = KEY_F3;
            else if (fNum == 14) targetKey = KEY_F4;
            else if (fNum == 15) targetKey = KEY_F5;
            else if (fNum == 17) targetKey = KEY_F6;
            else if (fNum == 18) targetKey = KEY_F7;
            else if (fNum == 19) targetKey = KEY_F8;
            else if (fNum == 20) targetKey = KEY_F9;
            else if (fNum == 21) targetKey = KEY_F10;
            else if (fNum == 23) targetKey = KEY_F11;
            else if (fNum == 24) targetKey = KEY_F12;

            if (targetKey != 0)
            {
                event.key = targetKey;
                event.modifiers = m;
                return event;
            }
        }
        if (strcmp(seq, "OP") == 0 || strcmp(seq, "[11~") == 0) { event.key = KEY_F1; return event; }
        if (strcmp(seq, "OQ") == 0 || strcmp(seq, "[12~") == 0) { event.key = KEY_F2; return event; }
        if (strcmp(seq, "OR") == 0 || strcmp(seq, "[13~") == 0) { event.key = KEY_F3; return event; }
        if (strcmp(seq, "OS") == 0 || strcmp(seq, "[14~") == 0) { event.key = KEY_F4; return event; }

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