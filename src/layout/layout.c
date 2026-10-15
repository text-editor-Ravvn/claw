#include "layout.h"
#include "config.h"

LayoutState layoutState = {0};

void layoutInit(void)
{
    layoutState.showLineNumbers = configShowLineNumbers();
    layoutState.showStatusBar = configShowStatusBar();
    layoutState.showWelcome = configShowWelcome();

    layoutState.centeredMode = 0;
    layoutState.wideMode = 0;
}

void layoutToggleLineNumbers(void)
{
    layoutState.showLineNumbers = !layoutState.showLineNumbers;
}

void layoutToggleStatusBar(void)
{
    layoutState.showStatusBar = !layoutState.showStatusBar;
}

void layoutToggleWelcome(void)
{
    layoutState.showWelcome = !layoutState.showWelcome;
}

void layoutToggleCenteredMode(void)
{
    layoutState.centeredMode = !layoutState.centeredMode;
}

void layoutToggleWideMode(void)
{
    layoutState.wideMode = !layoutState.wideMode;
}
