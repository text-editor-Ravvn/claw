#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_manager.h"
#include "editor.h"
#include "viewport.h"
#include "fileio.h"
#include "highlight.h"
#include "git.h"
#include "split.h"

BufferManager bufferManager = {0};
static int nextBufferId = 1;

extern Buffer buffer;
extern Cursor cursor;
extern Viewport viewport;
extern char *currentFile;

static void extractBaseName(const char *path, char *out, size_t outSize)
{
    if (!path || !*path)
    {
        snprintf(out, outSize, "[Untitled]");
        return;
    }

    const char *lastSlash = strrchr(path, '/');
    const char *lastBackslash = strrchr(path, '\\');
    const char *base = path;

    if (lastSlash && lastSlash > base)
        base = lastSlash + 1;
    if (lastBackslash && lastBackslash > base)
        base = lastBackslash + 1;

    snprintf(out, outSize, "%s", base);
}

static void freeDocRows(Buffer *doc)
{
    if (!doc || !doc->rows)
        return;

    for (int i = 0; i < doc->numRows; i++)
    {
        free(doc->rows[i].chars);
        free(doc->rows[i].hl);
    }
    free(doc->rows);
    doc->rows = NULL;
    doc->numRows = 0;
}

static void initEmptyDoc(Buffer *doc)
{
    doc->numRows = 1;
    doc->endsWithNewline = 0;
    doc->modified = 0;
    doc->rows = malloc(sizeof(Row));
    if (!doc->rows)
    {
        doc->numRows = 0;
        return;
    }
    doc->rows[0].size = 0;
    doc->rows[0].chars = malloc(1);
    if (!doc->rows[0].chars)
    {
        free(doc->rows);
        doc->rows = NULL;
        doc->numRows = 0;
        return;
    }
    doc->rows[0].chars[0] = '\0';
    doc->rows[0].hl = NULL;
    doc->rows[0].hlSize = 0;
    doc->rows[0].hlOpenState = 0;
}

void bufferManagerInit(void)
{
    if (buffer.rows != NULL)
    {
        int isManaged = 0;
        for (int i = 0; i < bufferManager.count; i++)
        {
            if (buffer.rows == bufferManager.buffers[i].doc.rows)
            {
                isManaged = 1;
                break;
            }
        }
        if (!isManaged)
        {
            bufferFree();
        }
    }
    bufferManagerFreeAll();
    bufferManager.count = 1;
    bufferManager.activeIndex = 0;

    FileBuffer *fb = &bufferManager.buffers[0];
    fb->id = nextBufferId++;
    fb->active = 1;
    fb->filename[0] = '\0';
    snprintf(fb->displayName, sizeof(fb->displayName), "[Untitled]");
    initEmptyDoc(&fb->doc);
    fb->cursor.x = 0;
    fb->cursor.y = 0;
    fb->rowOffset = 0;
    fb->colOffset = 0;

    /* Sync active slot to global state */
    bufferManagerSyncToGlobal();
}

int bufferManagerCount(void)
{
    return bufferManager.count;
}

int bufferManagerActiveIndex(void)
{
    return bufferManager.activeIndex;
}

FileBuffer *bufferManagerGet(int index)
{
    if (index < 0 || index >= bufferManager.count)
        return NULL;
    return &bufferManager.buffers[index];
}

FileBuffer *bufferManagerGetActive(void)
{
    if (bufferManager.activeIndex < 0 || bufferManager.activeIndex >= bufferManager.count)
        return NULL;
    return &bufferManager.buffers[bufferManager.activeIndex];
}

int bufferManagerFind(const char *filename)
{
    if (!filename || !*filename)
        return -1;

    for (int i = 0; i < bufferManager.count; i++)
    {
        if (strcmp(bufferManager.buffers[i].filename, filename) == 0)
            return i;
    }
    return -1;
}

void bufferManagerSyncFromGlobal(void)
{
    if (bufferManager.activeIndex < 0 || bufferManager.activeIndex >= bufferManager.count)
        return;

    FileBuffer *fb = &bufferManager.buffers[bufferManager.activeIndex];
    fb->doc = buffer;
    fb->cursor = cursor;
    fb->rowOffset = viewport.rowOffset;
    fb->colOffset = viewport.colOffset;

    if (currentFile && currentFile[0] != '\0')
    {
        snprintf(fb->filename, sizeof(fb->filename), "%s", currentFile);
        extractBaseName(currentFile, fb->displayName, sizeof(fb->displayName));
    }
}

void bufferManagerSyncToGlobal(void)
{
    if (bufferManager.activeIndex < 0 || bufferManager.activeIndex >= bufferManager.count)
        return;

    FileBuffer *fb = &bufferManager.buffers[bufferManager.activeIndex];
    buffer = fb->doc;
    cursor = fb->cursor;
    viewport.rowOffset = fb->rowOffset;
    viewport.colOffset = fb->colOffset;

    if (currentFile)
    {
        free(currentFile);
        currentFile = NULL;
    }

    if (fb->filename[0] != '\0')
    {
        currentFile = strdup(fb->filename);
    }
}

int bufferManagerSwitchTo(int index)
{
    if (index < 0 || index >= bufferManager.count || index == bufferManager.activeIndex)
        return 0;

    bufferManagerSyncFromGlobal();
    bufferManager.activeIndex = index;
    bufferManagerSyncToGlobal();

    highlightInit();
    gitRefreshStatus(currentFile);
    return 1;
}

int bufferManagerNext(void)
{
    if (bufferManager.count <= 1)
        return 0;

    int nextIdx = (bufferManager.activeIndex + 1) % bufferManager.count;
    return bufferManagerSwitchTo(nextIdx);
}

int bufferManagerPrev(void)
{
    if (bufferManager.count <= 1)
        return 0;

    int prevIdx = (bufferManager.activeIndex - 1 + bufferManager.count) % bufferManager.count;
    return bufferManagerSwitchTo(prevIdx);
}

int bufferManagerNew(const char *optionalName)
{
    if (bufferManager.count >= BUFFER_MAX_COUNT)
        return -1;

    bufferManagerSyncFromGlobal();

    int newIdx = bufferManager.count;
    FileBuffer *fb = &bufferManager.buffers[newIdx];
    memset(fb, 0, sizeof(*fb));
    fb->id = nextBufferId++;
    fb->active = 1;

    if (optionalName && *optionalName)
    {
        snprintf(fb->filename, sizeof(fb->filename), "%s", optionalName);
        extractBaseName(optionalName, fb->displayName, sizeof(fb->displayName));
    }
    else
    {
        fb->filename[0] = '\0';
        snprintf(fb->displayName, sizeof(fb->displayName), "[Untitled-%d]", fb->id);
    }

    initEmptyDoc(&fb->doc);
    fb->cursor.x = 0;
    fb->cursor.y = 0;
    fb->rowOffset = 0;
    fb->colOffset = 0;

    bufferManager.count++;
    bufferManager.activeIndex = newIdx;
    bufferManagerSyncToGlobal();

    highlightInit();
    gitRefreshStatus(currentFile);
    return newIdx;
}

int bufferManagerOpen(const char *filename)
{
    if (!filename || !*filename)
        return -1;

    /* Check if already open */
    int existing = bufferManagerFind(filename);
    if (existing >= 0)
    {
        bufferManagerSwitchTo(existing);
        return existing;
    }

    bufferManagerSyncFromGlobal();

    /* If current buffer is single, untitled, empty, and unmodified, reuse it */
    FileBuffer *cur = bufferManagerGetActive();
    int targetIdx = -1;
    if (bufferManager.count == 1 && cur && cur->filename[0] == '\0' &&
        cur->doc.modified == 0 && cur->doc.numRows == 1 && cur->doc.rows[0].size == 0)
    {
        targetIdx = 0;
        freeDocRows(&cur->doc);
    }
    else if (bufferManager.count < BUFFER_MAX_COUNT)
    {
        targetIdx = bufferManager.count;
        bufferManager.count++;
    }
    else
    {
        return -1; /* Maximum buffers reached */
    }

    bufferManager.activeIndex = targetIdx;
    FileBuffer *fb = &bufferManager.buffers[targetIdx];
    memset(fb, 0, sizeof(*fb));
    fb->id = nextBufferId++;
    fb->active = 1;
    snprintf(fb->filename, sizeof(fb->filename), "%s", filename);
    extractBaseName(filename, fb->displayName, sizeof(fb->displayName));
    initEmptyDoc(&fb->doc);
    fb->cursor.x = 0;
    fb->cursor.y = 0;
    fb->rowOffset = 0;
    fb->colOffset = 0;

    bufferManagerSyncToGlobal();

    if (openFile(filename))
    {
        bufferManagerSyncFromGlobal();
        highlightInit();
        gitRefreshStatus(currentFile);
        return targetIdx;
    }

    /* Fallback: open failed, keep empty doc with filename */
    return targetIdx;
}

int bufferManagerClose(int index)
{
    if (index < 0 || index >= bufferManager.count)
        return 0;

    bufferManagerSyncFromGlobal();

    FileBuffer *closing = &bufferManager.buffers[index];
    freeDocRows(&closing->doc);

    if (bufferManager.count == 1)
    {
        /* Last buffer: re-initialize as clean untitled */
        closing->id = nextBufferId++;
        closing->active = 1;
        closing->filename[0] = '\0';
        snprintf(closing->displayName, sizeof(closing->displayName), "[Untitled]");
        initEmptyDoc(&closing->doc);
        closing->cursor.x = 0;
        closing->cursor.y = 0;
        closing->rowOffset = 0;
        closing->colOffset = 0;
        bufferManager.activeIndex = 0;
        bufferManagerSyncToGlobal();
        highlightInit();
        gitRefreshStatus(NULL);
        return 1;
    }

    /* Shift subsequent buffers left */
    for (int i = index; i < bufferManager.count - 1; i++)
    {
        bufferManager.buffers[i] = bufferManager.buffers[i + 1];
    }
    memset(&bufferManager.buffers[bufferManager.count - 1], 0, sizeof(FileBuffer));
    bufferManager.count--;

    if (bufferManager.activeIndex >= bufferManager.count)
    {
        bufferManager.activeIndex = bufferManager.count - 1;
    }
    else if (index < bufferManager.activeIndex)
    {
        bufferManager.activeIndex--;
    }

    /* Update split panes if active */
    if (splitIsActive())
    {
        for (int p = 0; p < 2; p++)
        {
            if (splitLayout.panes[p].bufferIndex == index)
            {
                splitLayout.panes[p].bufferIndex = bufferManager.activeIndex;
            }
            else if (splitLayout.panes[p].bufferIndex > index)
            {
                splitLayout.panes[p].bufferIndex--;
            }
        }
    }

    bufferManagerSyncToGlobal();
    highlightInit();
    gitRefreshStatus(currentFile);
    return 1;
}

int bufferManagerCloseActive(void)
{
    return bufferManagerClose(bufferManager.activeIndex);
}

void bufferManagerFreeAll(void)
{
    for (int i = 0; i < bufferManager.count; i++)
    {
        freeDocRows(&bufferManager.buffers[i].doc);
    }
    memset(&bufferManager, 0, sizeof(bufferManager));
    buffer.rows = NULL;
    buffer.numRows = 0;
}

const char *bufferManagerGetDisplayName(int index)
{
    FileBuffer *fb = bufferManagerGet(index);
    if (!fb)
        return "[Unknown]";
    return fb->displayName;
}

/* Weak fallback definitions when highlight.c is not linked in standalone unit tests */
__attribute__((weak)) const char *highlightLanguageName(void)
{
    return "Plain Text";
}

__attribute__((weak)) const SyntaxDef *highlightFindLanguageByFilename(const char *filename)
{
    (void)filename;
    return NULL;
}

__attribute__((weak)) const SyntaxDef *highlightDetectLanguageFromContent(const char *firstLine)
{
    (void)firstLine;
    return NULL;
}

const char *bufferManagerGetLanguage(int index)
{
    if (index < 0 || index >= bufferManager.count)
        return "Plain Text";

    FileBuffer *fb = &bufferManager.buffers[index];
    if (index == bufferManager.activeIndex)
        return highlightLanguageName();

    if (fb->filename[0] != '\0')
    {
        const SyntaxDef *def = highlightFindLanguageByFilename(fb->filename);
        if (def)
            return def->name;
    }

    if (fb->doc.numRows > 0 && fb->doc.rows && fb->doc.rows[0].chars)
    {
        const SyntaxDef *def = highlightDetectLanguageFromContent(fb->doc.rows[0].chars);
        if (def)
            return def->name;
    }

    return "Plain Text";
}

