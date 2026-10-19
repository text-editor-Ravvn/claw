#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#include <io.h>
static DWORD saved_console_mode;
static int restore_console_mode = 0;
static DWORD saved_input_mode;
static int restore_input_mode = 0;
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
static struct termios saved_tty;
static int restore_tty = 0;
#endif

TerminalSize terminal_get_size(void) {
    TerminalSize s = {80, 24, 0, 0};
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (h != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(h, &info)) {
        s.cols = (int)(info.srWindow.Right - info.srWindow.Left + 1);
        s.rows = (int)(info.srWindow.Bottom - info.srWindow.Top + 1);
    }
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_col > 0) s.cols = (int)ws.ws_col;
        if (ws.ws_row > 0) s.rows = (int)ws.ws_row;
        s.pixel_width = ws.ws_xpixel;
        s.pixel_height = ws.ws_ypixel;
    }
#endif
    if (s.cols < 1) s.cols = 80;
    if (s.rows < 1) s.rows = 24;
    return s;
}

double terminal_cell_aspect(double fallback) {
#ifdef _WIN32
    (void)fallback;
    return 2.0;
#else
    TerminalSize s = terminal_get_size();
    if (s.rows > 0 && s.cols > 0 && s.pixel_width > 0 && s.pixel_height > 0) {
        double a = ((double)s.pixel_height / s.rows) / ((double)s.pixel_width / s.cols);
        if (a >= 1.2 && a <= 3.2) return a;
    }
    return fallback;
#endif
}

int terminal_is_interactive(void) {
#ifdef _WIN32
    return _isatty(_fileno(stdin)) && _isatty(_fileno(stdout));
#else
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
#endif
}

int terminal_enable_ansi(void) {
#ifndef _WIN32
    const char *term_name = getenv("TERM");
    if (term_name && !strcmp(term_name, "dumb")) return 0;
#endif
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode)) return 0;
    if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        if (!SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return 0;
    }
    saved_console_mode = mode;
    restore_console_mode = 1;
#endif
    return 1;
}

static int env_exists(const char *name) { return getenv(name) != NULL; }

ColorMode terminal_detect_color(ColorMode requested) {
    if (requested != COLOR_AUTO) return requested;
    if (env_exists("NO_COLOR") || !terminal_is_interactive()) return COLOR_NONE;
    const char *ct = getenv("COLORTERM");
    const char *term = getenv("TERM");
    const char *prog = getenv("TERM_PROGRAM");
    if ((ct && (strstr(ct, "truecolor") || strstr(ct, "24bit"))) ||
        env_exists("WT_SESSION") || (prog &&
        (!strcmp(prog, "iTerm.app") || !strcmp(prog, "vscode") ||
         !strcmp(prog, "WezTerm") || !strcmp(prog, "ghostty")))) return COLOR_TRUECOLOR;
    if (term && strstr(term, "256color")) return COLOR_256;
    if ((!term || !strcmp(term, "dumb")) && !env_exists("WT_SESSION")) return COLOR_NONE;
    return COLOR_16;
}

void terminal_begin_live(void) {
#ifndef _WIN32
    if (tcgetattr(STDIN_FILENO, &saved_tty) == 0) {
        struct termios raw = saved_tty;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) restore_tty = 1;
    }
#else
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)) {
        saved_input_mode = mode;
        restore_input_mode = 1;
        SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    }
#endif
    fputs("\033[?1049h\033[?25l", stdout);
    fflush(stdout);
}

void terminal_end_live(void) {
    fputs("\033[0m\033[?25h\033[?1049l", stdout);
    fflush(stdout);
#ifndef _WIN32
    if (restore_tty) tcsetattr(STDIN_FILENO, TCSADRAIN, &saved_tty);
#else
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    if (restore_input_mode && h != INVALID_HANDLE_VALUE) SetConsoleMode(h, saved_input_mode);
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (restore_console_mode && out != INVALID_HANDLE_VALUE) SetConsoleMode(out, saved_console_mode);
#endif
}

int terminal_key_pressed(void) {
#ifdef _WIN32
    if (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 224) { (void)_getch(); return 0; }
        return c == ' ' || c == 'q' || c == 'Q' || c == 27 || c == '\r' || c == 3;
    }
    return 0;
#else
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    return n == 1 && (c == ' ' || c == 'q' || c == 'Q' || c == 27 || c == '\r' || c == '\n' || c == 3);
#endif
}

void terminal_sleep_ms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000U);
    ts.tv_nsec = (long)(ms % 1000U) * 1000000L;
    while (nanosleep(&ts, &ts) != 0) { /* retry after interruption */ }
#endif
}

const char *terminal_reset_code(void) { return "\033[0m"; }
