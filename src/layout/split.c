#include <stdio.h>
#include <string.h>

#include "split.h"
#include "buffer_manager.h"
#include "viewport.h"
#include "cursor.h"
#include "editor.h"
#include "terminal.h"

SplitLayout splitLayout = {0};

extern Viewport viewport;
extern Cursor cursor;

void splitInit(void)
{
    memset(&splitLayout, 0, sizeof(splitLayout));
    splitLayout.mode = SPLIT_NONE;
    splitLayout.activePane = 0;
}

void splitUpdateDimensions(int screenRows, int screenCols)
{
    int termRows = terminalIsActive() ? integratedTerminal.height : 0;
    int editRows = screenRows - 2 - termRows;
    if (editRows < 2)
        editRows = 2;
    if (screenCols < 4)
        screenCols = 4;

    if (splitLayout.mode == SPLIT_NONE)
    {
        splitLayout.panes[0].x = 1;
        splitLayout.panes[0].y = 1;
        splitLayout.panes[0].width = screenCols;
        splitLayout.panes[0].height = editRows;
        return;
    }

    if (splitLayout.mode == SPLIT_HORIZONTAL)
    {
        int half = (editRows - 1) / 2;
        if (half < 1)
            half = 1;

        /* Top Pane (Pane 0) */
        splitLayout.panes[0].x = 1;
        splitLayout.panes[0].y = 1;
        splitLayout.panes[0].width = screenCols;
        splitLayout.panes[0].height = half;

        /* Divider is at row (1 + half) */

        /* Bottom Pane (Pane 1) */
        splitLayout.panes[1].x = 1;
        splitLayout.panes[1].y = 1 + half + 1;
        splitLayout.panes[1].width = screenCols;
        splitLayout.panes[1].height = editRows - half - 1;
        if (splitLayout.panes[1].height < 1)
            splitLayout.panes[1].height = 1;
    }
    else if (splitLayout.mode == SPLIT_VERTICAL)
    {
        int half = (screenCols - 1) / 2;
        if (half < 1)
            half = 1;

        /* Left Pane (Pane 0) */
        splitLayout.panes[0].x = 1;
        splitLayout.panes[0].y = 1;
        splitLayout.panes[0].width = half;
        splitLayout.panes[0].height = editRows;

        /* Divider is at column (1 + half) */

        /* Right Pane (Pane 1) */
        splitLayout.panes[1].x = 1 + half + 1;
        splitLayout.panes[1].y = 1;
        splitLayout.panes[1].width = screenCols - half - 1;
        splitLayout.panes[1].height = editRows;
        if (splitLayout.panes[1].width < 1)
            splitLayout.panes[1].width = 1;
    }
}

void splitHorizontal(void)
{
    bufferManagerSyncFromGlobal();

    splitLayout.mode = SPLIT_HORIZONTAL;
    splitLayout.panes[0].bufferIndex = bufferManagerActiveIndex();
    splitLayout.panes[0].cursorX = cursor.x;
    splitLayout.panes[0].cursorY = cursor.y;
    splitLayout.panes[0].rowOffset = viewport.rowOffset;
    splitLayout.panes[0].colOffset = viewport.colOffset;

    /* Assign second buffer to Pane 1 if available, or create/reuse */
    int bCount = bufferManagerCount();
    if (bCount > 1)
    {
        int other = (bufferManagerActiveIndex() + 1) % bCount;
        splitLayout.panes[1].bufferIndex = other;
        FileBuffer *fb = bufferManagerGet(other);
        if (fb)
        {
            splitLayout.panes[1].cursorX = fb->cursor.x;
            splitLayout.panes[1].cursorY = fb->cursor.y;
            splitLayout.panes[1].rowOffset = fb->rowOffset;
            splitLayout.panes[1].colOffset = fb->colOffset;
        }
    }
    else
    {
        splitLayout.panes[1].bufferIndex = splitLayout.panes[0].bufferIndex;
        splitLayout.panes[1].cursorX = 0;
        splitLayout.panes[1].cursorY = 0;
        splitLayout.panes[1].rowOffset = 0;
        splitLayout.panes[1].colOffset = 0;
    }

    splitLayout.activePane = 0;
    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    editorSetStatusMessage("Split view: Horizontal pane opened (Ctrl+W to close, Alt+O to switch)");
}

void splitVertical(void)
{
    bufferManagerSyncFromGlobal();

    splitLayout.mode = SPLIT_VERTICAL;
    splitLayout.panes[0].bufferIndex = bufferManagerActiveIndex();
    splitLayout.panes[0].cursorX = cursor.x;
    splitLayout.panes[0].cursorY = cursor.y;
    splitLayout.panes[0].rowOffset = viewport.rowOffset;
    splitLayout.panes[0].colOffset = viewport.colOffset;

    int bCount = bufferManagerCount();
    if (bCount > 1)
    {
        int other = (bufferManagerActiveIndex() + 1) % bCount;
        splitLayout.panes[1].bufferIndex = other;
        FileBuffer *fb = bufferManagerGet(other);
        if (fb)
        {
            splitLayout.panes[1].cursorX = fb->cursor.x;
            splitLayout.panes[1].cursorY = fb->cursor.y;
            splitLayout.panes[1].rowOffset = fb->rowOffset;
            splitLayout.panes[1].colOffset = fb->colOffset;
        }
    }
    else
    {
        splitLayout.panes[1].bufferIndex = splitLayout.panes[0].bufferIndex;
        splitLayout.panes[1].cursorX = 0;
        splitLayout.panes[1].cursorY = 0;
        splitLayout.panes[1].rowOffset = 0;
        splitLayout.panes[1].colOffset = 0;
    }

    splitLayout.activePane = 0;
    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    editorSetStatusMessage("Split view: Vertical pane opened (Ctrl+W to close, Alt+O to switch)");
}

void splitClose(void)
{
    if (splitLayout.mode == SPLIT_NONE)
        return;

    bufferManagerSyncFromGlobal();
    int remainingPane = 0;
    int targetBuffer = splitLayout.panes[remainingPane].bufferIndex;

    splitLayout.mode = SPLIT_NONE;
    splitLayout.activePane = 0;

    if (bufferManagerActiveIndex() != targetBuffer)
    {
        bufferManagerSwitchTo(targetBuffer);
    }
    cursor.x = splitLayout.panes[remainingPane].cursorX;
    cursor.y = splitLayout.panes[remainingPane].cursorY;
    viewport.rowOffset = splitLayout.panes[remainingPane].rowOffset;
    viewport.colOffset = splitLayout.panes[remainingPane].colOffset;

    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    editorSetStatusMessage("Split pane closed");
}

void splitSwitchPane(void)
{
    if (splitLayout.mode == SPLIT_NONE)
        return;

    /* Save outgoing pane state */
    bufferManagerSyncFromGlobal();
    EditorPane *out = &splitLayout.panes[splitLayout.activePane];
    out->cursorX = cursor.x;
    out->cursorY = cursor.y;
    out->rowOffset = viewport.rowOffset;
    out->colOffset = viewport.colOffset;
    out->bufferIndex = bufferManagerActiveIndex();

    /* Switch to incoming pane */
    splitLayout.activePane = 1 - splitLayout.activePane;
    EditorPane *in = &splitLayout.panes[splitLayout.activePane];

    bufferManagerSwitchTo(in->bufferIndex);
    cursor.x = in->cursorX;
    cursor.y = in->cursorY;
    viewport.rowOffset = in->rowOffset;
    viewport.colOffset = in->colOffset;

    /* Clamp cursor to document bounds */
    if (buffer.numRows > 0)
    {
        if (cursor.y >= buffer.numRows)
            cursor.y = buffer.numRows - 1;
        if (cursor.y < 0)
            cursor.y = 0;
        int rowLen = buffer.rows[cursor.y].size;
        if (cursor.x > rowLen)
            cursor.x = rowLen;
        if (cursor.x < 0)
            cursor.x = 0;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Active Pane: %s (%s)",
             splitLayout.activePane == 0 ? "Pane 1" : "Pane 2",
             bufferManagerGetDisplayName(in->bufferIndex));
    editorSetStatusMessage(msg);
}

int splitIsActive(void)
{
    return splitLayout.mode != SPLIT_NONE;
}

SplitMode splitGetMode(void)
{
    return splitLayout.mode;
}

EditorPane *splitGetPane(int index)
{
    if (index < 0 || index > 1)
        return NULL;
    return &splitLayout.panes[index];
}

EditorPane *splitGetActivePane(void)
{
    return &splitLayout.panes[splitLayout.activePane];
}
