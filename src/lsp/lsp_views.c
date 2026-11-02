/* Claw Intelligent IDE Services — LSP Interactive Views & Modals (Phase 11) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lsp.h"
#include "buffer.h"
#include "cursor.h"
#include "editor.h"
#include "viewport.h"
#include "render.h"
#include "input.h"
#include "buffer_manager.h"
#include "highlight.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

/* --- Global Modal States --- */
LspCompletionState lspCompletionMenu = {0};
LspHoverState lspHoverTooltip = {0};
LspReferencesState lspReferencesOverlay = {0};
LspCodeActionsState lspCodeActionsModal = {0};

/* ========================================================================= */
/* 1. Interactive Completion Menu (Ctrl+Space)                               */
/* ========================================================================= */

void lspCompletionOpen(void)
{
    lspCompletionMenu.active = 1;
    lspCompletionMenu.queryLen = 0;
    lspCompletionMenu.query[0] = '\0';
    lspCompletionMenu.selectedIndex = 0;
    lspCompletionMenu.triggerRow = cursor.y;
    lspCompletionMenu.triggerCol = cursor.x;

    /* Extract current prefix before cursor */
    if (cursor.y >= 0 && cursor.y < buffer.numRows)
    {
        Row *r = &buffer.rows[cursor.y];
        int start = cursor.x;
        while (start > 0 && (isalnum((unsigned char)r->chars[start - 1]) || r->chars[start - 1] == '_'))
            start--;

        int len = cursor.x - start;
        if (len > 0 && len < (int)sizeof(lspCompletionMenu.query))
        {
            memcpy(lspCompletionMenu.query, &r->chars[start], (size_t)len);
            lspCompletionMenu.query[len] = '\0';
            lspCompletionMenu.queryLen = len;
            lspCompletionMenu.triggerCol = start;
        }
    }

    lspCompletionMenu.count = lspRequestCompletion(
        currentFile ? currentFile : "",
        cursor.y + 1, cursor.x + 1,
        lspCompletionMenu.query,
        lspCompletionMenu.items,
        LSP_MAX_COMPLETIONS
    );

    if (lspCompletionMenu.count == 0)
    {
        lspCompletionMenu.active = 0;
        editorSetStatusMessage("No completions available");
    }
}

void lspCompletionClose(void)
{
    lspCompletionMenu.active = 0;
    lspCompletionMenu.count = 0;
    lspCompletionMenu.selectedIndex = 0;
}

int lspCompletionIsActive(void)
{
    return lspCompletionMenu.active;
}

static const char *getKindBadge(int kind)
{
    switch (kind)
    {
        case LSP_KIND_FUNCTION: return "[fn]  ";
        case LSP_KIND_METHOD:   return "[meth]";
        case LSP_KIND_VARIABLE: return "[var] ";
        case LSP_KIND_CLASS:    return "[cls] ";
        case LSP_KIND_STRUCT:   return "[str] ";
        case LSP_KIND_KEYWORD:  return "[kw]  ";
        case LSP_KIND_SNIPPET:  return "[snip]";
        default:                return "[text]";
    }
}

void lspCompletionDraw(void)
{
    if (!lspCompletionMenu.active || lspCompletionMenu.count == 0)
        return;

    int screenRow = cursor.y - viewport.rowOffset + 2;
    int screenCol = cursor.x - viewport.colOffset + 6;

    if (screenRow + 8 >= viewport.screenRows)
        screenRow = cursor.y - viewport.rowOffset - 8;
    if (screenRow < 2)
        screenRow = 2;
    if (screenCol + 45 >= viewport.screenCols)
        screenCol = viewport.screenCols - 45;
    if (screenCol < 1)
        screenCol = 1;

    int visibleCount = (lspCompletionMenu.count < 7) ? lspCompletionMenu.count : 7;
    int width = 46;

    /* Draw box header */
    printf("\033[%d;%dH\033[1;36m+------------------ Completions -------------------+\033[0m", screenRow, screenCol);

    int start = 0;
    if (lspCompletionMenu.selectedIndex >= visibleCount)
        start = lspCompletionMenu.selectedIndex - visibleCount + 1;

    for (int i = 0; i < visibleCount; i++)
    {
        int idx = start + i;
        if (idx >= lspCompletionMenu.count)
            break;

        LspCompletionItem *item = &lspCompletionMenu.items[idx];
        printf("\033[%d;%dH\033[1;36m|\033[0m", screenRow + 1 + i, screenCol);

        if (idx == lspCompletionMenu.selectedIndex)
            printf("\033[7m");

        printf(" %s %-20.20s %-16.16s",
               getKindBadge(item->kind),
               item->label,
               item->detail[0] ? item->detail : "");

        if (idx == lspCompletionMenu.selectedIndex)
            printf("\033[0m");

        printf("\033[%d;%dH\033[1;36m|\033[0m", screenRow + 1 + i, screenCol + width + 5);
    }

    printf("\033[%d;%dH\033[1;36m+--------------------------------------------------+\033[0m",
           screenRow + 1 + visibleCount, screenCol);
}

void lspCompletionHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27) /* Esc */
    {
        lspCompletionClose();
        return;
    }

    if (key == ARROW_UP)
    {
        if (lspCompletionMenu.selectedIndex > 0)
            lspCompletionMenu.selectedIndex--;
        else
            lspCompletionMenu.selectedIndex = lspCompletionMenu.count - 1;
        return;
    }

    if (key == ARROW_DOWN)
    {
        if (lspCompletionMenu.selectedIndex < lspCompletionMenu.count - 1)
            lspCompletionMenu.selectedIndex++;
        else
            lspCompletionMenu.selectedIndex = 0;
        return;
    }

    if (key == '\t' || key == '\r' || key == '\n')
    {
        if (lspCompletionMenu.selectedIndex >= 0 && lspCompletionMenu.selectedIndex < lspCompletionMenu.count)
        {
            LspCompletionItem *sel = &lspCompletionMenu.items[lspCompletionMenu.selectedIndex];
            const char *insertStr = sel->insertText[0] ? sel->insertText : sel->label;

            /* Delete currently typed prefix if any */
            if (lspCompletionMenu.queryLen > 0 && cursor.y >= 0 && cursor.y < buffer.numRows)
            {
                Row *row = &buffer.rows[cursor.y];
                for (int d = 0; d < lspCompletionMenu.queryLen && cursor.x > 0; d++)
                {
                    deleteChar();
                }
                (void)row;
            }

            /* Insert chosen text */
            for (size_t c = 0; c < strlen(insertStr); c++)
            {
                insertChar(insertStr[c]);
            }
        }
        lspCompletionClose();
        return;
    }

    if (key == 127 || key == 8)
    {
        if (lspCompletionMenu.queryLen > 0)
        {
            lspCompletionMenu.query[--lspCompletionMenu.queryLen] = '\0';
            deleteChar();
            lspCompletionMenu.count = lspRequestCompletion(
                currentFile ? currentFile : "",
                cursor.y + 1, cursor.x + 1,
                lspCompletionMenu.query,
                lspCompletionMenu.items,
                LSP_MAX_COMPLETIONS
            );
            lspCompletionMenu.selectedIndex = 0;
            if (lspCompletionMenu.count == 0)
                lspCompletionClose();
        }
        else
        {
            lspCompletionClose();
            deleteChar();
        }
        return;
    }

    if (key >= 32 && key <= 126)
    {
        if (lspCompletionMenu.queryLen < (int)sizeof(lspCompletionMenu.query) - 1)
        {
            lspCompletionMenu.query[lspCompletionMenu.queryLen++] = (char)key;
            lspCompletionMenu.query[lspCompletionMenu.queryLen] = '\0';
            insertChar((char)key);
            lspCompletionMenu.count = lspRequestCompletion(
                currentFile ? currentFile : "",
                cursor.y + 1, cursor.x + 1,
                lspCompletionMenu.query,
                lspCompletionMenu.items,
                LSP_MAX_COMPLETIONS
            );
            lspCompletionMenu.selectedIndex = 0;
            if (lspCompletionMenu.count == 0)
                lspCompletionClose();
        }
        return;
    }

    lspCompletionClose();
}

/* ========================================================================= */
/* 2. Hover Tooltip (Alt+K)                                                  */
/* ========================================================================= */

void lspHoverOpen(void)
{
    lspHoverTooltip.active = 1;
    lspHoverTooltip.line = cursor.y + 1;
    lspHoverTooltip.col = cursor.x + 1;
    lspHoverTooltip.signature[0] = '\0';
    lspHoverTooltip.documentation[0] = '\0';

    if (!lspRequestHover(currentFile ? currentFile : "",
                         lspHoverTooltip.line, lspHoverTooltip.col,
                         lspHoverTooltip.signature, sizeof(lspHoverTooltip.signature),
                         lspHoverTooltip.documentation, sizeof(lspHoverTooltip.documentation)))
    {
        lspHoverTooltip.active = 0;
        editorSetStatusMessage("No hover information available");
    }
}

void lspHoverClose(void)
{
    lspHoverTooltip.active = 0;
}

int lspHoverIsActive(void)
{
    return lspHoverTooltip.active;
}

void lspHoverDraw(void)
{
    if (!lspHoverTooltip.active)
        return;

    int width = 64;
    int startCol = (viewport.screenCols - width) / 2;
    if (startCol < 1) startCol = 1;
    int startRow = 6;

    printf("\033[%d;%dH\033[1;33m+------------------------ Hover Info --------------------------+\033[0m", startRow++, startCol);
    printf("\033[%d;%dH\033[1;33m|\033[0m \033[1;37m%-60.60s\033[0m \033[1;33m|\033[0m", startRow++, startCol, lspHoverTooltip.signature);
    printf("\033[%d;%dH\033[1;33m+--------------------------------------------------------------+\033[0m", startRow++, startCol);

    /* Print documentation (up to 4 lines) */
    const char *doc = lspHoverTooltip.documentation;
    for (int l = 0; l < 4; l++)
    {
        char lineBuf[61] = {0};
        if (doc && *doc)
        {
            int len = 0;
            while (doc[len] && doc[len] != '\n' && len < 60)
                len++;
            memcpy(lineBuf, doc, (size_t)len);
            lineBuf[len] = '\0';
            doc += len;
            if (*doc == '\n') doc++;
        }
        printf("\033[%d;%dH\033[1;33m|\033[0m %-60.60s \033[1;33m|\033[0m", startRow++, startCol, lineBuf);
    }

    printf("\033[%d;%dH\033[1;33m+------------------------ [Esc] Close -------------------------+\033[0m", startRow, startCol);
}

void lspHoverHandleKey(int key, int modifiers)
{
    (void)modifiers;
    (void)key;
    lspHoverClose();
}

/* ========================================================================= */
/* 3. Find References Provider & Overlay (CMD_LSP_REFERENCES)                */
/* ========================================================================= */

void lspReferencesOpen(void)
{
    lspReferencesOverlay.active = 1;
    lspReferencesOverlay.selectedIndex = 0;
    lspReferencesOverlay.scrollOffset = 0;
    lspReferencesOverlay.count = lspRequestReferences(
        currentFile ? currentFile : "",
        cursor.y + 1, cursor.x + 1,
        lspReferencesOverlay.locations,
        LSP_MAX_LOCATIONS
    );

    if (lspReferencesOverlay.count == 0)
    {
        lspReferencesOverlay.active = 0;
        editorSetStatusMessage("No references found");
    }
}

void lspReferencesClose(void)
{
    lspReferencesOverlay.active = 0;
    lspReferencesOverlay.count = 0;
}

int lspReferencesIsActive(void)
{
    return lspReferencesOverlay.active;
}

void lspReferencesDraw(void)
{
    if (!lspReferencesOverlay.active)
        return;

    printf("\033[2J\033[H");
    printf("\033[1;36m=== Symbol References (Claw IDE) ===\033[0m\r\n");
    printf("\033[90mFound %d reference%s across project\033[0m\r\n",
           lspReferencesOverlay.count, lspReferencesOverlay.count == 1 ? "" : "s");
    printf("----------------------------------------------------------------------\r\n");

    int maxDisplay = viewport.screenRows - 6;
    if (maxDisplay < 1) maxDisplay = 1;

    int start = 0;
    if (lspReferencesOverlay.selectedIndex >= maxDisplay)
        start = lspReferencesOverlay.selectedIndex - maxDisplay + 1;

    for (int i = 0; i < maxDisplay; i++)
    {
        int idx = start + i;
        if (idx >= lspReferencesOverlay.count)
            break;

        LspLocation *loc = &lspReferencesOverlay.locations[idx];
        if (idx == lspReferencesOverlay.selectedIndex)
            printf("\033[7m");

        printf(" %c %-28.28s:%-4d  %-35.35s\033[0m\r\n",
               (idx == lspReferencesOverlay.selectedIndex) ? '>' : ' ',
               loc->file, loc->line, loc->preview);
    }

    printf("\033[%d;1H\033[1;36m[Enter] Jump  [Up/Down] Navigate  [Esc] Close\033[0m\r\n",
           viewport.screenRows);
}

void lspReferencesHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27)
    {
        lspReferencesClose();
        return;
    }

    if (key == ARROW_UP)
    {
        if (lspReferencesOverlay.selectedIndex > 0)
            lspReferencesOverlay.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN)
    {
        if (lspReferencesOverlay.selectedIndex < lspReferencesOverlay.count - 1)
            lspReferencesOverlay.selectedIndex++;
        return;
    }

    if (key == '\r' || key == '\n')
    {
        if (lspReferencesOverlay.selectedIndex >= 0 && lspReferencesOverlay.selectedIndex < lspReferencesOverlay.count)
        {
            LspLocation *loc = &lspReferencesOverlay.locations[lspReferencesOverlay.selectedIndex];
            if (loc->file[0])
            {
                if (!currentFile || strcmp(currentFile, loc->file) != 0)
                {
                    bufferManagerOpen(loc->file);
                }
                if (loc->line > 0 && loc->line <= buffer.numRows)
                {
                    cursor.y = loc->line - 1;
                    cursor.x = loc->col > 0 ? loc->col - 1 : 0;
                    viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
                    if (viewport.rowOffset < 0)
                        viewport.rowOffset = 0;
                }
            }
        }
        lspReferencesClose();
        return;
    }
}

/* ========================================================================= */
/* 4. Code Actions & Refactoring Modal (Alt+Enter)                           */
/* ========================================================================= */

void lspCodeActionsOpen(void)
{
    lspCodeActionsModal.active = 1;
    lspCodeActionsModal.selectedIndex = 0;
    lspCodeActionsModal.count = lspRequestCodeActions(
        currentFile ? currentFile : "",
        cursor.y + 1, cursor.x + 1,
        lspCodeActionsModal.actions,
        LSP_MAX_CODE_ACTIONS
    );

    if (lspCodeActionsModal.count == 0)
    {
        lspCodeActionsModal.active = 0;
        editorSetStatusMessage("No code actions available at cursor");
    }
}

void lspCodeActionsClose(void)
{
    lspCodeActionsModal.active = 0;
    lspCodeActionsModal.count = 0;
}

int lspCodeActionsIsActive(void)
{
    return lspCodeActionsModal.active;
}

void lspCodeActionsDraw(void)
{
    if (!lspCodeActionsModal.active || lspCodeActionsModal.count == 0)
        return;

    int width = 56;
    int startCol = (viewport.screenCols - width) / 2;
    if (startCol < 1) startCol = 1;
    int startRow = 6;

    printf("\033[%d;%dH\033[1;35m+---------------- QuickFix & Refactor -----------------+\033[0m", startRow++, startCol);

    for (int i = 0; i < lspCodeActionsModal.count && i < 6; i++)
    {
        LspCodeAction *act = &lspCodeActionsModal.actions[i];
        printf("\033[%d;%dH\033[1;35m|\033[0m", startRow + i, startCol);

        if (i == lspCodeActionsModal.selectedIndex)
            printf("\033[7m");

        printf(" %d. %-48.48s", i + 1, act->title);

        if (i == lspCodeActionsModal.selectedIndex)
            printf("\033[0m");

        printf("\033[%d;%dH\033[1;35m|\033[0m", startRow + i, startCol + width);
    }

    startRow += (lspCodeActionsModal.count < 6 ? lspCodeActionsModal.count : 6);
    printf("\033[%d;%dH\033[1;35m+----------- [Enter] Apply  [Esc] Cancel --------------+\033[0m", startRow, startCol);
}

void lspCodeActionsHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27)
    {
        lspCodeActionsClose();
        return;
    }

    if (key == ARROW_UP)
    {
        if (lspCodeActionsModal.selectedIndex > 0)
            lspCodeActionsModal.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN)
    {
        if (lspCodeActionsModal.selectedIndex < lspCodeActionsModal.count - 1)
            lspCodeActionsModal.selectedIndex++;
        return;
    }

    if (key == '\r' || key == '\n')
    {
        lspApplyCodeAction(lspCodeActionsModal.selectedIndex);
        lspCodeActionsClose();
        return;
    }

    if (key >= '1' && key <= '9')
    {
        int idx = key - '1';
        if (idx < lspCodeActionsModal.count)
        {
            lspApplyCodeAction(idx);
            lspCodeActionsClose();
            return;
        }
    }
}
