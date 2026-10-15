#ifndef LAYOUT_H
#define LAYOUT_H

typedef struct
{
    int showLineNumbers;
    int showStatusBar;
    int showWelcome;
    int centeredMode;
    int wideMode;
} LayoutState;

extern LayoutState layoutState;

void layoutInit(void);

void layoutToggleLineNumbers(void);
void layoutToggleStatusBar(void);
void layoutToggleWelcome(void);
void layoutToggleCenteredMode(void);
void layoutToggleWideMode(void);

#endif