#include "history.h"

static EditAction history[HISTORY_MAX];

static int historyCount = 0;

void historyInit(void)
{
    historyCount = 0;
}