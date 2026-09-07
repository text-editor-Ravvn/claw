#include "history.h"

#include "buffer.h"
#include "cursor.h"

extern Buffer buffer;
extern Cursor cursor;

static EditAction history[HISTORY_MAX];

static int historyCount = 0;

void historyInit(void)
{
    historyCount = 0;
}

void historyPushInsert(
    int row,
    int col,
    char ch
)
{
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
}

int historyUndo(void)
{
    if (historyCount == 0)
        return 0;

    EditAction action =
        history[historyCount - 1];

    historyCount--;

    if (action.type ==
        ACTION_INSERT)
    {
        cursor.y =
            action.row;

        cursor.x =
            action.col + 1;

        deleteChar();

        return 1;
    }

    if (action.type ==
        ACTION_DELETE)
    {
        cursor.y =
            action.row;

        cursor.x =
            action.col;

        insertChar(
            action.ch
        );

        cursor.x =
            action.col + 1;

        return 1;
    }

    return 0;
}
void historyPushDelete(
    int row,
    int col,
    char ch
)
{
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
}