#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct
{
    int screenRows;
    int screenCols;

    int rowOffset;
    int colOffset;

    /* Width of the line-number gutter (0 when line numbers are hidden). */
    int gutterWidth;

} Viewport;

extern Viewport viewport;

void viewportInit(void);

void scrollEditor(void);

void viewportUpdateSize(void);
#endif