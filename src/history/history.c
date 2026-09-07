#include "history.h"

#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;
extern Cursor cursor;

static EditAction history[HISTORY_MAX];
static int historyCount = 0;

static EditAction redoHistory[HISTORY_MAX];
static int redoCount = 0;

static int recordingEnabled = 1;

void historyInit(void)
{
    historyCount = 0;
    redoCount = 0;
}

void historySetRecording(int enabled)
{
    recordingEnabled = enabled;
}

void historyPushInsert(
    int row,
    int col,
    char ch
)
{
    if (!recordingEnabled)
        return;

    if (historyCount >= HISTORY_MAX)
        return;

    history[historyCount].type =
        ACTION_INSERT;

    history[historyCount].row =
        row;

    history[historyCount].col =
        col;

    history[historyCount].ch =
        ch;

    historyCount++;

    redoCount = 0;
}

void historyPushDelete(
    int row,
    int col,
    char ch
)
{
    if (!recordingEnabled)
        return;

    if (historyCount >= HISTORY_MAX)
        return;

    history[historyCount].type =
        ACTION_DELETE;

    history[historyCount].row =
        row;

    history[historyCount].col =
        col;

    history[historyCount].ch =
        ch;

    historyCount++;

    redoCount = 0;
}

int historyUndo(void)
{
    if (historyCount == 0)
        return 0;

    EditAction action =
        history[historyCount - 1];

    historyCount--;

    if (redoCount < HISTORY_MAX)
        redoHistory[redoCount++] = action;

    historySetRecording(0);

    if (action.type == ACTION_INSERT)
{
    cursor.y = action.row;
    cursor.x = action.col + 1;

    deleteChar();
}
else if (action.type == ACTION_DELETE)
{
    cursor.y = action.row;
    cursor.x = action.col;

    insertChar(action.ch);
}
else if (action.type == ACTION_NEWLINE)
{
    cursor.y = action.row + 1;
    cursor.x = 0;

    deleteChar();
}

    historySetRecording(1);

    return 1;
}

void historyRedo(void)
{
    if (redoCount == 0)
        return;

    EditAction action =
        redoHistory[redoCount - 1];

    redoCount--;

    historySetRecording(0);

    if (action.type == ACTION_INSERT)
{
    cursor.y = action.row;
    cursor.x = action.col;

    insertChar(action.ch);
}
else if (action.type == ACTION_DELETE)
{
    cursor.y = action.row;
    cursor.x = action.col + 1;

    deleteChar();
}
else if (action.type == ACTION_NEWLINE)
{
    cursor.y = action.row;
    cursor.x = action.col;

    insertNewLine();
}
    historySetRecording(1);

    if (historyCount < HISTORY_MAX)
        history[historyCount++] = action;
}
void historyPushNewLine(
    int row,
    int col
)
{
    if (!recordingEnabled)
        return;

    if (historyCount >= HISTORY_MAX)
        return;

    history[historyCount].type =
        ACTION_NEWLINE;

    history[historyCount].row =
        row;

    history[historyCount].col =
        col;

    history[historyCount].ch =
        '\0';

    historyCount++;

    redoCount = 0;
}