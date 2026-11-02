#ifndef SPLIT_H
#define SPLIT_H

typedef enum
{
    SPLIT_NONE = 0,
    SPLIT_HORIZONTAL, /* Two panes stacked vertically (top and bottom) */
    SPLIT_VERTICAL    /* Two panes side-by-side (left and right) */
} SplitMode;

typedef struct
{
    int bufferIndex; /* Index of buffer in bufferManager shown in this pane */
    int x;           /* Screen column (1-based) */
    int y;           /* Screen row (1-based) */
    int width;       /* Width in columns */
    int height;      /* Height in rows */
    int cursorX;
    int cursorY;
    int rowOffset;
    int colOffset;
} EditorPane;

typedef struct
{
    SplitMode mode;
    EditorPane panes[2];
    int activePane;  /* 0 or 1 */
} SplitLayout;

extern SplitLayout splitLayout;

/* Initialize split screen subsystem */
void splitInit(void);

/* Split the viewport horizontally into top and bottom panes */
void splitHorizontal(void);

/* Split the viewport vertically into left and right panes */
void splitVertical(void);

/* Close active split, returning to single-pane layout */
void splitClose(void);

/* Switch focus to the other pane (0 <-> 1) */
void splitSwitchPane(void);

/* Returns 1 if split screen is active, 0 otherwise */
int splitIsActive(void);

/* Get current split mode */
SplitMode splitGetMode(void);

/* Get pane by index (0 or 1) */
EditorPane *splitGetPane(int index);

/* Get currently active pane */
EditorPane *splitGetActivePane(void);

/* Recalculate pane dimensions based on terminal screen size */
void splitUpdateDimensions(int screenRows, int screenCols);

#endif
