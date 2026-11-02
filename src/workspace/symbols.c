#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "symbols.h"
#include "buffer_manager.h"
#include "workspace.h"
#include "recent.h"
#include "editor.h"
#include "cursor.h"
#include "viewport.h"
#include "fileio.h"
#include "highlight.h"
#include "git.h"

SymbolIndexState symbolState = {0};

extern Buffer buffer;
extern Cursor cursor;
extern Viewport viewport;
extern char *currentFile;

void symbolsInit(void)
{
    memset(&symbolState, 0, sizeof(symbolState));
}

void symbolsClear(void)
{
    symbolState.count = 0;
    symbolState.filteredCount = 0;
    symbolState.selectedIndex = 0;
    symbolState.query[0] = '\0';
    symbolState.queryLen = 0;
}

const char *symbolKindToString(SymbolKind kind)
{
    switch (kind)
    {
        case SYMBOL_FUNCTION: return "func";
        case SYMBOL_STRUCT:   return "struct";
        case SYMBOL_ENUM:     return "enum";
        case SYMBOL_MACRO:    return "macro";
        case SYMBOL_TYPEDEF:  return "type";
        case SYMBOL_VARIABLE: return "var";
        default:              return "sym";
    }
}

void symbolsAdd(const char *name, const char *file, int line, SymbolKind kind, const char *sig)
{
    if (!name || !*name || symbolState.count >= SYMBOLS_MAX)
        return;

    Symbol *s = &symbolState.symbols[symbolState.count++];
    snprintf(s->name, sizeof(s->name), "%s", name);
    snprintf(s->file, sizeof(s->file), "%s", file ? file : "");
    s->line = line;
    s->kind = kind;
    snprintf(s->signature, sizeof(s->signature), "%s", sig ? sig : name);
}

static void trimLine(const char *in, char *out, size_t outSize)
{
    while (*in == ' ' || *in == '\t') in++;
    snprintf(out, outSize, "%s", in);
    size_t len = strlen(out);
    while (len > 0 && (out[len - 1] == ' ' || out[len - 1] == '\t' || out[len - 1] == '\r' || out[len - 1] == '\n'))
    {
        out[--len] = '\0';
    }
}

void symbolsIndexBuffer(const Buffer *buf, const char *filename)
{
    if (!buf || !buf->rows)
        return;

    const char *fName = filename ? filename : "[Untitled]";

    for (int i = 0; i < buf->numRows; i++)
    {
        if (symbolState.count >= SYMBOLS_MAX)
            break;

        const char *line = buf->rows[i].chars;
        if (!line || !*line)
            continue;

        char trimmed[256];
        trimLine(line, trimmed, sizeof(trimmed));

        /* 1. Macro: #define NAME */
        if (strncmp(trimmed, "#define", 7) == 0 && (trimmed[7] == ' ' || trimmed[7] == '\t'))
        {
            const char *p = trimmed + 7;
            while (*p == ' ' || *p == '\t') p++;
            char name[SYMBOL_NAME_MAX];
            int nLen = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_') && nLen < SYMBOL_NAME_MAX - 1)
            {
                name[nLen++] = *p++;
            }
            name[nLen] = '\0';
            if (nLen > 0)
            {
                symbolsAdd(name, fName, i + 1, SYMBOL_MACRO, trimmed);
            }
            continue;
        }

        /* 2. Struct: struct NAME { or typedef struct ... */
        if (strncmp(trimmed, "struct", 6) == 0 && (trimmed[6] == ' ' || trimmed[6] == '\t'))
        {
            const char *p = trimmed + 6;
            while (*p == ' ' || *p == '\t') p++;
            char name[SYMBOL_NAME_MAX];
            int nLen = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_') && nLen < SYMBOL_NAME_MAX - 1)
            {
                name[nLen++] = *p++;
            }
            name[nLen] = '\0';
            if (nLen > 0)
            {
                symbolsAdd(name, fName, i + 1, SYMBOL_STRUCT, trimmed);
            }
            continue;
        }

        /* 3. Enum: enum NAME */
        if (strncmp(trimmed, "enum", 4) == 0 && (trimmed[4] == ' ' || trimmed[4] == '\t'))
        {
            const char *p = trimmed + 4;
            while (*p == ' ' || *p == '\t') p++;
            char name[SYMBOL_NAME_MAX];
            int nLen = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_') && nLen < SYMBOL_NAME_MAX - 1)
            {
                name[nLen++] = *p++;
            }
            name[nLen] = '\0';
            if (nLen > 0)
            {
                symbolsAdd(name, fName, i + 1, SYMBOL_ENUM, trimmed);
            }
            continue;
        }

        /* 4. Typedef: typedef ... NAME; */
        if (strncmp(trimmed, "typedef", 7) == 0)
        {
            char *lastWord = strrchr(trimmed, ' ');
            if (!lastWord) lastWord = strrchr(trimmed, '\t');
            if (lastWord)
            {
                lastWord++;
                char name[SYMBOL_NAME_MAX];
                int nLen = 0;
                while (*lastWord && (isalnum((unsigned char)*lastWord) || *lastWord == '_') && nLen < SYMBOL_NAME_MAX - 1)
                {
                    name[nLen++] = *lastWord++;
                }
                name[nLen] = '\0';
                if (nLen > 0)
                {
                    symbolsAdd(name, fName, i + 1, SYMBOL_TYPEDEF, trimmed);
                }
            }
            continue;
        }

        /* 5. Python def/class */
        if (strncmp(trimmed, "def ", 4) == 0)
        {
            const char *p = trimmed + 4;
            char name[SYMBOL_NAME_MAX];
            int nLen = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_') && nLen < SYMBOL_NAME_MAX - 1)
            {
                name[nLen++] = *p++;
            }
            name[nLen] = '\0';
            if (nLen > 0)
            {
                symbolsAdd(name, fName, i + 1, SYMBOL_FUNCTION, trimmed);
            }
            continue;
        }
        if (strncmp(trimmed, "class ", 6) == 0)
        {
            const char *p = trimmed + 6;
            char name[SYMBOL_NAME_MAX];
            int nLen = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_') && nLen < SYMBOL_NAME_MAX - 1)
            {
                name[nLen++] = *p++;
            }
            name[nLen] = '\0';
            if (nLen > 0)
            {
                symbolsAdd(name, fName, i + 1, SYMBOL_STRUCT, trimmed);
            }
            continue;
        }

        /* 6. C/C++ Function detection: has '(' and does not start with keyword */
        if (strchr(trimmed, '(') != NULL && strchr(trimmed, ')') != NULL)
        {
            int hasLeadingWs = (line[0] == ' ' || line[0] == '\t');
            size_t tLen = strlen(trimmed);
            int endsWithSemi = (tLen > 0 && trimmed[tLen - 1] == ';');
            const char *paren = strchr(trimmed, '(');
            const char *eq = strchr(trimmed, '=');

            /* Skip assignments before parens and indented statements ending with semicolon */
            if ((!eq || eq > paren) && !(hasLeadingWs && endsWithSemi) &&
                strncmp(trimmed, "if ", 3) != 0 && strncmp(trimmed, "if(", 3) != 0 &&
                strncmp(trimmed, "while ", 6) != 0 && strncmp(trimmed, "while(", 6) != 0 &&
                strncmp(trimmed, "for ", 4) != 0 && strncmp(trimmed, "for(", 4) != 0 &&
                strncmp(trimmed, "switch ", 7) != 0 && strncmp(trimmed, "switch(", 7) != 0 &&
                strncmp(trimmed, "return ", 7) != 0 && strncmp(trimmed, "return(", 7) != 0 &&
                trimmed[0] != '/' && trimmed[0] != '*' && trimmed[0] != '#')
            {
                const char *end = paren - 1;
                while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '*')) end--;

                const char *start = end;
                while (start > trimmed && (isalnum((unsigned char)*(start - 1)) || *(start - 1) == '_')) start--;

                int nLen = (int)(end - start + 1);
                if (nLen > 0 && nLen < SYMBOL_NAME_MAX)
                {
                    char name[SYMBOL_NAME_MAX];
                    strncpy(name, start, (size_t)nLen);
                    name[nLen] = '\0';

                    /* Ignore keywords */
                    if (strcmp(name, "if") != 0 && strcmp(name, "while") != 0 &&
                        strcmp(name, "for") != 0 && strcmp(name, "sizeof") != 0 &&
                        strcmp(name, "switch") != 0 && strcmp(name, "return") != 0)
                    {
                        symbolsAdd(name, fName, i + 1, SYMBOL_FUNCTION, trimmed);
                    }
                }
            }
        }
    }
}

void symbolsIndexWorkspace(void)
{
    symbolsClear();

    /* 1. Index active buffer */
    bufferManagerSyncFromGlobal();
    symbolsIndexBuffer(&buffer, currentFile);

    /* 2. Index all open buffers in buffer manager */
    int bCount = bufferManagerCount();
    for (int i = 0; i < bCount; i++)
    {
        if (i == bufferManagerActiveIndex())
            continue;
        FileBuffer *fb = bufferManagerGet(i);
        if (fb)
        {
            symbolsIndexBuffer(&fb->doc, fb->filename);
        }
    }

    /* 3. Index workspace files if active and space permits */
    if (workspaceIsActive() && workspaceGetRoot() && symbolState.count < SYMBOLS_MAX)
    {
        DIR *dir = opendir(workspaceGetRoot());
        if (dir)
        {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL && symbolState.count < SYMBOLS_MAX)
            {
                if (entry->d_name[0] == '.')
                    continue;
                const char *dot = strrchr(entry->d_name, '.');
                if (dot && (strcmp(dot, ".c") == 0 || strcmp(dot, ".h") == 0 ||
                            strcmp(dot, ".py") == 0 || strcmp(dot, ".cpp") == 0))
                {
                    char full[512];
                    snprintf(full, sizeof(full), "%s/%s", workspaceGetRoot(), entry->d_name);
                    if (bufferManagerFind(full) < 0 && (currentFile == NULL || strcmp(currentFile, full) != 0))
                    {
                        FILE *fp = fopen(full, "r");
                        if (fp)
                        {
                            char fline[512];
                            int lineNo = 1;
                            while (fgets(fline, sizeof(fline), fp) && symbolState.count < SYMBOLS_MAX)
                            {
                                char tr[256];
                                trimLine(fline, tr, sizeof(tr));
                                if (strncmp(tr, "#define", 7) == 0 && (tr[7] == ' ' || tr[7] == '\t'))
                                {
                                    const char *p = tr + 7;
                                    while (*p == ' ' || *p == '\t') p++;
                                    char sname[SYMBOL_NAME_MAX];
                                    int slen = 0;
                                    while (*p && (isalnum((unsigned char)*p) || *p == '_') && slen < SYMBOL_NAME_MAX - 1)
                                        sname[slen++] = *p++;
                                    sname[slen] = '\0';
                                    if (slen > 0)
                                        symbolsAdd(sname, full, lineNo, SYMBOL_MACRO, tr);
                                }
                                else if (strncmp(tr, "struct", 6) == 0 && (tr[6] == ' ' || tr[6] == '\t'))
                                {
                                    const char *p = tr + 6;
                                    while (*p == ' ' || *p == '\t') p++;
                                    char sname[SYMBOL_NAME_MAX];
                                    int slen = 0;
                                    while (*p && (isalnum((unsigned char)*p) || *p == '_') && slen < SYMBOL_NAME_MAX - 1)
                                        sname[slen++] = *p++;
                                    sname[slen] = '\0';
                                    if (slen > 0)
                                        symbolsAdd(sname, full, lineNo, SYMBOL_STRUCT, tr);
                                }
                                else if (fline[0] != ' ' && fline[0] != '\t' && strchr(tr, '(') && strchr(tr, ')') &&
                                         tr[0] != '/' && tr[0] != '*' && tr[0] != '#')
                                {
                                    const char *paren = strchr(tr, '(');
                                    const char *eq = strchr(tr, '=');
                                    if (!eq || eq > paren)
                                    {
                                        const char *end = paren - 1;
                                        while (end > tr && (*end == ' ' || *end == '\t' || *end == '*')) end--;
                                        const char *start = end;
                                        while (start > tr && (isalnum((unsigned char)*(start - 1)) || *(start - 1) == '_')) start--;
                                        int nLen = (int)(end - start + 1);
                                        if (nLen > 0 && nLen < SYMBOL_NAME_MAX)
                                        {
                                            char sname[SYMBOL_NAME_MAX];
                                            strncpy(sname, start, (size_t)nLen);
                                            sname[nLen] = '\0';
                                            if (strcmp(sname, "if") != 0 && strcmp(sname, "while") != 0 &&
                                                strcmp(sname, "for") != 0 && strcmp(sname, "sizeof") != 0 &&
                                                strcmp(sname, "switch") != 0 && strcmp(sname, "return") != 0)
                                            {
                                                symbolsAdd(sname, full, lineNo, SYMBOL_FUNCTION, tr);
                                            }
                                        }
                                    }
                                }
                                lineNo++;
                            }
                            fclose(fp);
                        }
                    }
                }
            }
            closedir(dir);
        }
    }

    symbolsFilter();
}

static int containsSubstr(const char *haystack, const char *needle)
{
    if (!needle || !*needle)
        return 1;
    if (!haystack || !*haystack)
        return 0;

    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen)
        return 0;

    for (size_t i = 0; i <= hlen - nlen; i++)
    {
        size_t j = 0;
        while (j < nlen && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j]))
        {
            j++;
        }
        if (j == nlen)
            return 1;
    }
    return 0;
}

void symbolsFilter(void)
{
    symbolState.filteredCount = 0;

    for (int i = 0; i < symbolState.count; i++)
    {
        if (symbolState.queryLen == 0 ||
            containsSubstr(symbolState.symbols[i].name, symbolState.query) ||
            containsSubstr(symbolState.symbols[i].file, symbolState.query) ||
            containsSubstr(symbolState.symbols[i].signature, symbolState.query))
        {
            if (symbolState.filteredCount < SYMBOLS_MAX)
            {
                symbolState.filteredIndices[symbolState.filteredCount++] = i;
            }
        }
    }

    if (symbolState.selectedIndex >= symbolState.filteredCount)
    {
        symbolState.selectedIndex = symbolState.filteredCount > 0 ? symbolState.filteredCount - 1 : 0;
    }
}

void symbolsOpen(void)
{
    symbolState.active = 1;
    symbolsIndexWorkspace();
    editorSetStatusMessage("Symbol Navigator: Type to filter, Enter to jump, Esc to cancel");
}

void symbolsClose(void)
{
    symbolState.active = 0;
}

int symbolsIsActive(void)
{
    return symbolState.active;
}

void symbolsSelectNext(void)
{
    if (symbolState.filteredCount == 0)
        return;
    if (symbolState.selectedIndex < symbolState.filteredCount - 1)
        symbolState.selectedIndex++;
}

void symbolsSelectPrev(void)
{
    if (symbolState.filteredCount == 0)
        return;
    if (symbolState.selectedIndex > 0)
        symbolState.selectedIndex--;
}

void symbolsAddChar(char c)
{
    if (symbolState.queryLen < (int)sizeof(symbolState.query) - 1)
    {
        symbolState.query[symbolState.queryLen++] = c;
        symbolState.query[symbolState.queryLen] = '\0';
        symbolsFilter();
    }
}

void symbolsBackspace(void)
{
    if (symbolState.queryLen > 0)
    {
        symbolState.query[--symbolState.queryLen] = '\0';
        symbolsFilter();
    }
}

const Symbol *symbolsGetSelected(void)
{
    if (symbolState.filteredCount == 0 || symbolState.selectedIndex >= symbolState.filteredCount)
        return NULL;
    return &symbolState.symbols[symbolState.filteredIndices[symbolState.selectedIndex]];
}

int symbolsJumpToSelected(void)
{
    const Symbol *sym = symbolsGetSelected();
    if (!sym)
        return 0;

    char targetFile[SYMBOL_PATH_MAX];
    snprintf(targetFile, sizeof(targetFile), "%s", sym->file);
    int targetLine = sym->line;
    char symName[SYMBOL_NAME_MAX];
    snprintf(symName, sizeof(symName), "%s", sym->name);

    symbolsClose();

    if (targetFile[0] != '\0' && strcmp(targetFile, "[Untitled]") != 0)
    {
        int bufIdx = bufferManagerFind(targetFile);
        if (bufIdx >= 0)
        {
            bufferManagerSwitchTo(bufIdx);
        }
        else if (currentFile == NULL || strcmp(currentFile, targetFile) != 0)
        {
            bufferManagerOpen(targetFile);
        }
    }

    if (targetLine > 0)
    {
        if (buffer.numRows > 0)
        {
            cursor.y = (targetLine <= buffer.numRows) ? targetLine - 1 : buffer.numRows - 1;
        }
        else
        {
            cursor.y = targetLine - 1;
        }
        cursor.x = 0;
        viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
        if (viewport.rowOffset < 0)
            viewport.rowOffset = 0;
    }

    char status[700];
    snprintf(status, sizeof(status), "Jumped to %s (%s:%d)", symName, targetFile, targetLine);
    editorSetStatusMessage(status);
    return 1;
}
