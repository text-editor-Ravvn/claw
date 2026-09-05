#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct
{
    int screenRows;
    int screenCols;

    int rowOffset;
    int colOffset;

} Viewport;

extern Viewport viewport;

void viewportInit(void);

void scrollEditor(void);

void viewportUpdateSize(void);
#endif