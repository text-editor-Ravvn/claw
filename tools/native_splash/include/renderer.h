#ifndef CLAW_RENDERER_H
#define CLAW_RENDERER_H
#include "terminal.h"
int renderer_init(void);
void renderer_free(void);
void renderer_draw(int max_cols, int max_rows, double cell_aspect,
                   ColorMode color_mode, int center_vertically);
double renderer_image_aspect(void);
#endif
