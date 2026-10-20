#define _POSIX_C_SOURCE 200809L
#include "renderer.h"
#include "terminal.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t stop_requested = 0;
static void request_stop(int sig) { (void)sig; stop_requested = 1; }

typedef struct {
    int once;
    int force_watch;
    int cols;
    int rows;
    double aspect;
    ColorMode color;
} Options;

static void usage(FILE *f, const char *name) {
    fprintf(f,
        "CLAW terminal splash — native ASCII-art renderer\n\n"
        "Usage: %s [options]\n\n"
        "  --once             Draw one frame and exit\n"
        "  --watch            Stay open and redraw when the terminal is resized\n"
        "  --cols N           Force output width (also selects one-frame mode)\n"
        "  --rows N           Force output height (also selects one-frame mode)\n"
        "  --aspect X         Character-cell height/width ratio (default: detect, else 2.0)\n"
        "  --color MODE       auto, truecolor, 256, 16, or none\n"
        "  --help             Show this help\n\n"
        "Live mode: press q, Esc, Enter, or Ctrl+C to exit.\n", name);
}

static int parse_positive_int(const char *s, int *out) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (!s || !*s || !end || *end || v < 1 || v > 10000) return 0;
    *out = (int)v;
    return 1;
}

static int parse_options(int argc, char **argv, Options *o) {
    o->once = 0; o->force_watch = 0; o->cols = 0; o->rows = 0;
    o->aspect = 0.0; o->color = COLOR_AUTO;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(stdout, argv[0]); exit(EXIT_SUCCESS);
        } else if (!strcmp(argv[i], "--once")) o->once = 1;
        else if (!strcmp(argv[i], "--watch")) o->force_watch = 1;
        else if (!strcmp(argv[i], "--cols") || !strcmp(argv[i], "--rows")) {
            int is_cols = !strcmp(argv[i], "--cols");
            if (++i >= argc) { fprintf(stderr, "Missing value for %s\n", argv[i-1]); return 0; }
            int val;
            if (!parse_positive_int(argv[i], &val)) { fprintf(stderr, "Invalid size: %s\n", argv[i]); return 0; }
            if (is_cols) o->cols = val; else o->rows = val;
        } else if (!strcmp(argv[i], "--aspect")) {
            if (++i >= argc) { fputs("Missing value for --aspect\n", stderr); return 0; }
            char *end = NULL; double val = strtod(argv[i], &end);
            if (!argv[i][0] || !end || *end || val < 0.5 || val > 4.0) {
                fprintf(stderr, "Invalid aspect ratio: %s (use 0.5 to 4.0)\n", argv[i]); return 0;
            }
            o->aspect = val;
        } else if (!strcmp(argv[i], "--color")) {
            if (++i >= argc) { fputs("Missing value for --color\n", stderr); return 0; }
            if (!strcmp(argv[i], "auto")) o->color = COLOR_AUTO;
            else if (!strcmp(argv[i], "truecolor")) o->color = COLOR_TRUECOLOR;
            else if (!strcmp(argv[i], "256")) o->color = COLOR_256;
            else if (!strcmp(argv[i], "16")) o->color = COLOR_16;
            else if (!strcmp(argv[i], "none")) o->color = COLOR_NONE;
            else { fprintf(stderr, "Unknown color mode: %s\n", argv[i]); return 0; }
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 0;
        }
    }
    return 1;
}

static void draw_once(const Options *o, ColorMode color) {
    TerminalSize t = terminal_get_size();
    int cols = o->cols ? o->cols : t.cols;
    int rows = o->rows ? o->rows : (t.rows > 1 ? t.rows - 1 : 1);
    double aspect = o->aspect > 0.0 ? o->aspect : terminal_cell_aspect(2.0);
    renderer_draw(cols, rows, aspect, color, 0);
}

static void draw_live(const Options *o, ColorMode color) {
    signal(SIGINT, request_stop);
#ifdef SIGTERM
    signal(SIGTERM, request_stop);
#endif
    terminal_begin_live();
    int last_cols = -1, last_rows = -1;
    double last_aspect = -1.0;
    while (!stop_requested) {
        TerminalSize t = terminal_get_size();
        double aspect = o->aspect > 0.0 ? o->aspect : terminal_cell_aspect(2.0);
        if (t.cols != last_cols || t.rows != last_rows || aspect != last_aspect) {
            last_cols = t.cols; last_rows = t.rows; last_aspect = aspect;
            fputs("\033[0m\033[2J\033[H", stdout);
            if (t.cols < 12 || t.rows < 5) {
                const char *msg = "terminal too small";
                for (int i = 0; i < t.cols && msg[i]; ++i) putchar(msg[i]);
                putchar('\n'); fflush(stdout);
            } else {
                renderer_draw(t.cols, t.rows, aspect, color, 1);
            }
        }
        if (terminal_key_pressed()) break;
        terminal_sleep_ms(60);
    }
    terminal_end_live();
}

int main(int argc, char **argv) {
    Options options;
    if (!parse_options(argc, argv, &options)) { usage(stderr, argv[0]); return EXIT_FAILURE; }
    int ansi = terminal_enable_ansi();
    ColorMode color = ansi ? terminal_detect_color(options.color) : COLOR_NONE;
    if (!renderer_init()) {
        fputs("CLAW splash: embedded artwork could not be decoded.\n", stderr);
        return EXIT_FAILURE;
    }
    int interactive = terminal_is_interactive() && ansi;
    int once = options.once || options.cols || options.rows || !interactive;
    /* --watch asks for live mode, but still falls back safely when stdin/stdout are redirected. */
    if (options.force_watch && interactive && !options.once && !options.cols && !options.rows) once = 0;
    if (once) draw_once(&options, color);
    else draw_live(&options, color);
    renderer_free();
    return ferror(stdout) ? EXIT_FAILURE : EXIT_SUCCESS;
}
