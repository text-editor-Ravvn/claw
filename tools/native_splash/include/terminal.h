#ifndef CLAW_TERMINAL_H
#define CLAW_TERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int cols;
    int rows;
    unsigned int pixel_width;
    unsigned int pixel_height;
} TerminalSize;

typedef enum {
    COLOR_AUTO = -1,
    COLOR_NONE = 0,
    COLOR_TRUECOLOR = 1,
    COLOR_256 = 2,
    COLOR_16 = 3
} ColorMode;

TerminalSize terminal_get_size(void);
double terminal_cell_aspect(double fallback);
int terminal_is_interactive(void);
int terminal_enable_ansi(void);
ColorMode terminal_detect_color(ColorMode requested);
void terminal_begin_live(void);
void terminal_end_live(void);
int terminal_key_pressed(void);
void terminal_sleep_ms(unsigned int ms);
const char *terminal_reset_code(void);

#ifdef __cplusplus
}
#endif
#endif
