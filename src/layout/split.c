#include <stdio.h>
#include <string.h>

#include "split.h"
#include "buffer_manager.h"
#include "viewport.h"
#include "cursor.h"
#include "editor.h"
#include "terminal.h"
#include "render.h"

__attribute__((weak)) void renderForceClear(void)
{
}

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

    if (splitLayout.mode != SPLIT_NONE)
    {
        splitLayout.mode = SPLIT_HORIZONTAL;
        splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
        renderForceClear();
        editorSetStatusMessage("Split view: Horizontal pane active (Ctrl+W to close, Alt+O to switch)");
        return;
    }

    int savedX = cursor.x;
    int savedY = cursor.y;
    int savedRowOff = viewport.rowOffset;
    int savedColOff = viewport.colOffset;
    int origBuf = bufferManagerActiveIndex();

    splitLayout.mode = SPLIT_HORIZONTAL;
    splitLayout.panes[0].bufferIndex = origBuf;
    splitLayout.panes[0].cursorX = savedX;
    splitLayout.panes[0].cursorY = savedY;
    splitLayout.panes[0].rowOffset = savedRowOff;
    splitLayout.panes[0].colOffset = savedColOff;

    /* Assign second buffer to Pane 1 if available, or create a separate buffer */
    int bCount = bufferManagerCount();
    if (bCount > 1)
    {
        int other = (origBuf + 1) % bCount;
        splitLayout.panes[1].bufferIndex = other;
        FileBuffer *fb = bufferManagerGet(other);
        if (fb)
        {
            splitLayout.panes[1].cursorX = fb->cursor.x;
            splitLayout.panes[1].cursorY = fb->cursor.y;
            splitLayout.panes[1].rowOffset = fb->rowOffset;
            splitLayout.panes[1].colOffset = fb->colOffset;
        }
        else
        {
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;
        }
    }
    else
    {
        /* Temporarily disable split mode during bufferManagerNew so it does not overwrite Pane 0 */
        splitLayout.mode = SPLIT_NONE;
        int newIdx = bufferManagerNew(NULL);
        splitLayout.mode = SPLIT_HORIZONTAL;

        if (newIdx >= 0)
        {
            splitLayout.panes[1].bufferIndex = newIdx;
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;

            splitLayout.activePane = 0;
            bufferManagerSwitchTo(origBuf);
            splitLayout.panes[0].bufferIndex = origBuf;
            splitLayout.panes[0].cursorX = savedX;
            splitLayout.panes[0].cursorY = savedY;
            splitLayout.panes[0].rowOffset = savedRowOff;
            splitLayout.panes[0].colOffset = savedColOff;
            cursor.x = savedX;
            cursor.y = savedY;
            viewport.rowOffset = savedRowOff;
            viewport.colOffset = savedColOff;
        }
        else
        {
            splitLayout.panes[1].bufferIndex = origBuf;
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;
        }
    }

    splitLayout.activePane = 0;
    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    renderForceClear();
    editorSetStatusMessage("Split view: Horizontal pane opened (Ctrl+W to close, Alt+O to switch)");
}

void splitVertical(void)
{
    bufferManagerSyncFromGlobal();

    if (splitLayout.mode != SPLIT_NONE)
    {
        splitLayout.mode = SPLIT_VERTICAL;
        splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
        renderForceClear();
        editorSetStatusMessage("Split view: Vertical pane active (Ctrl+W to close, Alt+O to switch)");
        return;
    }

    int savedX = cursor.x;
    int savedY = cursor.y;
    int savedRowOff = viewport.rowOffset;
    int savedColOff = viewport.colOffset;
    int origBuf = bufferManagerActiveIndex();

    splitLayout.mode = SPLIT_VERTICAL;
    splitLayout.panes[0].bufferIndex = origBuf;
    splitLayout.panes[0].cursorX = savedX;
    splitLayout.panes[0].cursorY = savedY;
    splitLayout.panes[0].rowOffset = savedRowOff;
    splitLayout.panes[0].colOffset = savedColOff;

    int bCount = bufferManagerCount();
    if (bCount > 1)
    {
        int other = (origBuf + 1) % bCount;
        splitLayout.panes[1].bufferIndex = other;
        FileBuffer *fb = bufferManagerGet(other);
        if (fb)
        {
            splitLayout.panes[1].cursorX = fb->cursor.x;
            splitLayout.panes[1].cursorY = fb->cursor.y;
            splitLayout.panes[1].rowOffset = fb->rowOffset;
            splitLayout.panes[1].colOffset = fb->colOffset;
        }
        else
        {
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;
        }
    }
    else
    {
        /* Temporarily disable split mode during bufferManagerNew so it does not overwrite Pane 0 */
        splitLayout.mode = SPLIT_NONE;
        int newIdx = bufferManagerNew(NULL);
        splitLayout.mode = SPLIT_VERTICAL;

        if (newIdx >= 0)
        {
            splitLayout.panes[1].bufferIndex = newIdx;
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;

            splitLayout.activePane = 0;
            bufferManagerSwitchTo(origBuf);
            splitLayout.panes[0].bufferIndex = origBuf;
            splitLayout.panes[0].cursorX = savedX;
            splitLayout.panes[0].cursorY = savedY;
            splitLayout.panes[0].rowOffset = savedRowOff;
            splitLayout.panes[0].colOffset = savedColOff;
            cursor.x = savedX;
            cursor.y = savedY;
            viewport.rowOffset = savedRowOff;
            viewport.colOffset = savedColOff;
        }
        else
        {
            splitLayout.panes[1].bufferIndex = origBuf;
            splitLayout.panes[1].cursorX = 0;
            splitLayout.panes[1].cursorY = 0;
            splitLayout.panes[1].rowOffset = 0;
            splitLayout.panes[1].colOffset = 0;
        }
    }

    splitLayout.activePane = 0;
    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    renderForceClear();
    editorSetStatusMessage("Split view: Vertical pane opened (Ctrl+W to close, Alt+O to switch)");
}

void splitClose(void)
{
    if (splitLayout.mode == SPLIT_NONE)
        return;

    bufferManagerSyncFromGlobal();
    int remainingPane = splitLayout.activePane;

    /* Update current active pane coordinates before collapsing */
    splitLayout.panes[remainingPane].cursorX = cursor.x;
    splitLayout.panes[remainingPane].cursorY = cursor.y;
    splitLayout.panes[remainingPane].rowOffset = viewport.rowOffset;
    splitLayout.panes[remainingPane].colOffset = viewport.colOffset;
    splitLayout.panes[remainingPane].bufferIndex = bufferManagerActiveIndex();

    int targetBuffer = splitLayout.panes[remainingPane].bufferIndex;
    int savedX = splitLayout.panes[remainingPane].cursorX;
    int savedY = splitLayout.panes[remainingPane].cursorY;
    int savedRowOff = splitLayout.panes[remainingPane].rowOffset;
    int savedColOff = splitLayout.panes[remainingPane].colOffset;

    splitLayout.mode = SPLIT_NONE;
    splitLayout.activePane = 0;
    splitLayout.panes[0].bufferIndex = targetBuffer;
    splitLayout.panes[0].cursorX = savedX;
    splitLayout.panes[0].cursorY = savedY;
    splitLayout.panes[0].rowOffset = savedRowOff;
    splitLayout.panes[0].colOffset = savedColOff;

    if (bufferManagerActiveIndex() != targetBuffer)
    {
        bufferManagerSwitchTo(targetBuffer);
    }
    cursor.x = savedX;
    cursor.y = savedY;
    viewport.rowOffset = savedRowOff;
    viewport.colOffset = savedColOff;

    splitUpdateDimensions(viewport.screenRows, viewport.screenCols);
    renderForceClear();
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

    if (in->bufferIndex < 0 || in->bufferIndex >= bufferManagerCount())
    {
        in->bufferIndex = bufferManagerActiveIndex();
    }

    int targetX = in->cursorX;
    int targetY = in->cursorY;
    int targetRowOff = in->rowOffset;
    int targetColOff = in->colOffset;

    bufferManagerSwitchTo(in->bufferIndex);
    cursor.x = targetX;
    cursor.y = targetY;
    viewport.rowOffset = targetRowOff;
    viewport.colOffset = targetColOff;
    in->cursorX = targetX;
    in->cursorY = targetY;
    in->rowOffset = targetRowOff;
    in->colOffset = targetColOff;

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
    else
    {
        cursor.x = 0;
        cursor.y = 0;
    }

    if (viewport.rowOffset < 0)
        viewport.rowOffset = 0;
    if (viewport.colOffset < 0)
        viewport.colOffset = 0;

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
