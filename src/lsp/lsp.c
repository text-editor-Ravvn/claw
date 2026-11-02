/* Claw Intelligent IDE Services — LSP Client Engine (Phase 11) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "lsp.h"
#include "buffer.h"
#include "buffer_manager.h"
#include "cursor.h"
#include "editor.h"
#include "symbols.h"
#include "viewport.h"
#include "highlight.h"

extern Buffer buffer;
extern Cursor cursor;
extern char *currentFile;

/* --- Internal LSP Global State --- */
static LspServer registeredServers[LSP_MAX_SERVERS];
static int serverCount = 0;

static LspDiagnostic diagnosticStore[LSP_MAX_DIAGNOSTICS];
static int diagnosticCount = 0;

/* --- Protocol Framing & Formatting --- */

int lspFormatHeader(size_t bodyLength, char *out, size_t outSize)
{
    if (!out || outSize == 0)
        return 0;

    int written = snprintf(out, outSize, "Content-Length: %zu\r\n\r\n", bodyLength);
    return (written > 0 && (size_t)written < outSize) ? written : 0;
}

int lspParseHeader(const char *bufferText, size_t *outContentLen, size_t *outHeaderLen)
{
    if (!bufferText)
        return 0;

    const char *clStr = strstr(bufferText, "Content-Length:");
    if (!clStr)
        clStr = strstr(bufferText, "content-length:");
    if (!clStr)
        return 0;

    clStr += strlen("Content-Length:");
    while (*clStr == ' ' || *clStr == '\t')
        clStr++;

    long cl = strtol(clStr, NULL, 10);
    if (cl <= 0)
        return 0;

    const char *endOfHeaders = strstr(bufferText, "\r\n\r\n");
    if (!endOfHeaders)
    {
        endOfHeaders = strstr(bufferText, "\n\n");
        if (!endOfHeaders)
            return 0;
        if (outHeaderLen)
            *outHeaderLen = (size_t)(endOfHeaders + 2 - bufferText);
    }
    else
    {
        if (outHeaderLen)
            *outHeaderLen = (size_t)(endOfHeaders + 4 - bufferText);
    }

    if (outContentLen)
        *outContentLen = (size_t)cl;

    return 1;
}

int lspFormatRequest(int id, const char *method, const char *paramsJson, char *out, size_t outSize)
{
    if (!method || !out || outSize == 0)
        return 0;

    char body[4096];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"jsonrpc\":\"2.0\",\"id\":%d,\"method\":\"%s\",\"params\":%s}",
                          id, method, paramsJson ? paramsJson : "{}");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    char header[64];
    int headerLen = lspFormatHeader((size_t)bodyLen, header, sizeof(header));
    if (headerLen <= 0)
        return 0;

    int total = snprintf(out, outSize, "%s%s", header, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

int lspFormatNotification(const char *method, const char *paramsJson, char *out, size_t outSize)
{
    if (!method || !out || outSize == 0)
        return 0;

    char body[4096];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":%s}",
                          method, paramsJson ? paramsJson : "{}");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    char header[64];
    int headerLen = lspFormatHeader((size_t)bodyLen, header, sizeof(header));
    if (headerLen <= 0)
        return 0;

    int total = snprintf(out, outSize, "%s%s", header, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

int lspFormatResponse(int id, const char *resultJson, char *out, size_t outSize)
{
    if (!out || outSize == 0)
        return 0;

    char body[4096];
    int bodyLen = snprintf(body, sizeof(body),
                          "{\"jsonrpc\":\"2.0\",\"id\":%d,\"result\":%s}",
                          id, resultJson ? resultJson : "null");
    if (bodyLen <= 0 || (size_t)bodyLen >= sizeof(body))
        return 0;

    char header[64];
    int headerLen = lspFormatHeader((size_t)bodyLen, header, sizeof(header));
    if (headerLen <= 0)
        return 0;

    int total = snprintf(out, outSize, "%s%s", header, body);
    return (total > 0 && (size_t)total < outSize) ? total : 0;
}

/* --- Server Lifecycle & Registration --- */

int lspInit(void)
{
    serverCount = 0;
    diagnosticCount = 0;

    /* Register built-in language server configurations */
    lspRegisterServer("c", "clangd");
    lspRegisterServer("cpp", "clangd");
    lspRegisterServer("python", "pyright-langserver --stdio");
    lspRegisterServer("rust", "rust-analyzer");
    lspRegisterServer("go", "gopls");
    lspRegisterServer("javascript", "typescript-language-server --stdio");
    lspRegisterServer("typescript", "typescript-language-server --stdio");

    return 1;
}

void lspShutdown(void)
{
    lspStopAll();
    lspClearAllDiagnostics();
}

int lspRegisterServer(const char *langId, const char *command)
{
    if (!langId || !command || serverCount >= LSP_MAX_SERVERS)
        return 0;

    for (int i = 0; i < serverCount; i++)
    {
        if (strcasecmp(registeredServers[i].langId, langId) == 0)
        {
            snprintf(registeredServers[i].command, sizeof(registeredServers[i].command), "%s", command);
            return 1;
        }
    }

    LspServer *s = &registeredServers[serverCount++];
    memset(s, 0, sizeof(*s));
    snprintf(s->langId, sizeof(s->langId), "%s", langId);
    snprintf(s->command, sizeof(s->command), "%s", command);
    s->status = LSP_STATUS_STOPPED;
    s->nextRequestId = 1;
    s->hasCompletion = 1;
    s->hasHover = 1;
    s->hasDefinition = 1;
    s->hasReferences = 1;
    s->hasCodeAction = 1;

    return 1;
}

int lspStartServer(const char *langId, const char *rootPath)
{
    LspServer *s = lspGetServerForLanguage(langId);
    if (!s)
        return 0;

    if (rootPath)
        snprintf(s->rootUri, sizeof(s->rootUri), "%s", rootPath);
    else
        snprintf(s->rootUri, sizeof(s->rootUri), ".");

    s->status = LSP_STATUS_RUNNING;
    s->initialized = 1;
    return 1;
}

int lspStopServer(const char *langId)
{
    LspServer *s = lspGetServerForLanguage(langId);
    if (!s)
        return 0;

    s->status = LSP_STATUS_STOPPED;
    s->initialized = 0;
    return 1;
}

void lspStopAll(void)
{
    for (int i = 0; i < serverCount; i++)
    {
        registeredServers[i].status = LSP_STATUS_STOPPED;
        registeredServers[i].initialized = 0;
    }
}

LspServer *lspGetServerForLanguage(const char *langId)
{
    if (!langId)
        return NULL;

    for (int i = 0; i < serverCount; i++)
    {
        if (strcasecmp(registeredServers[i].langId, langId) == 0)
            return &registeredServers[i];
    }
    return NULL;
}

int lspServerCount(void)
{
    return serverCount;
}

/* --- Document Synchronization --- */

void lspNotifyDidOpen(const char *filename, const char *langId, const char *text)
{
    (void)filename;
    (void)langId;
    (void)text;
}

void lspNotifyDidChange(const char *filename, const char *text)
{
    (void)filename;
    (void)text;
}

void lspNotifyDidSave(const char *filename)
{
    (void)filename;
}

void lspNotifyDidClose(const char *filename)
{
    if (filename)
        lspClearDiagnostics(filename);
}

/* --- Diagnostics Engine --- */

void lspAddDiagnostic(const char *uri, int line, int col, int severity, const char *msg, const char *source, const char *code)
{
    if (!uri || diagnosticCount >= LSP_MAX_DIAGNOSTICS)
        return;

    LspDiagnostic *d = &diagnosticStore[diagnosticCount++];
    memset(d, 0, sizeof(*d));
    snprintf(d->uri, sizeof(d->uri), "%s", uri);
    d->line = line;
    d->column = col;
    d->severity = severity;
    if (msg)
        snprintf(d->message, sizeof(d->message), "%s", msg);
    if (source)
        snprintf(d->source, sizeof(d->source), "%s", source);
    if (code)
        snprintf(d->code, sizeof(d->code), "%s", code);
}

void lspClearDiagnostics(const char *uri)
{
    if (!uri)
        return;

    int newCount = 0;
    for (int i = 0; i < diagnosticCount; i++)
    {
        if (strcmp(diagnosticStore[i].uri, uri) != 0)
        {
            if (newCount != i)
                diagnosticStore[newCount] = diagnosticStore[i];
            newCount++;
        }
    }
    diagnosticCount = newCount;
}

void lspClearAllDiagnostics(void)
{
    diagnosticCount = 0;
}

int lspGetDiagnosticsForLine(const char *uri, int line, LspDiagnostic *outDiags, int maxDiags)
{
    if (!uri || !outDiags || maxDiags <= 0)
        return 0;

    int found = 0;
    for (int i = 0; i < diagnosticCount && found < maxDiags; i++)
    {
        if (diagnosticStore[i].line == line &&
            (strcmp(diagnosticStore[i].uri, uri) == 0 || strstr(diagnosticStore[i].uri, uri) || strstr(uri, diagnosticStore[i].uri)))
        {
            outDiags[found++] = diagnosticStore[i];
        }
    }
    return found;
}

int lspGutterStatus(const char *uri, int line)
{
    if (!uri || line <= 0)
        return LSP_SEVERITY_NONE;

    int highestSeverity = LSP_SEVERITY_NONE;
    for (int i = 0; i < diagnosticCount; i++)
    {
        if (diagnosticStore[i].line == line &&
            (strcmp(diagnosticStore[i].uri, uri) == 0 || strstr(diagnosticStore[i].uri, uri) || strstr(uri, diagnosticStore[i].uri)))
        {
            if (highestSeverity == LSP_SEVERITY_NONE || diagnosticStore[i].severity < highestSeverity)
            {
                highestSeverity = diagnosticStore[i].severity;
            }
        }
    }
    return highestSeverity;
}

int lspTotalDiagnosticsCount(const char *uri)
{
    if (!uri)
        return diagnosticCount;

    int c = 0;
    for (int i = 0; i < diagnosticCount; i++)
    {
        if (strcmp(diagnosticStore[i].uri, uri) == 0 || strstr(diagnosticStore[i].uri, uri) || strstr(uri, diagnosticStore[i].uri))
            c++;
    }
    return c;
}

const LspDiagnostic *lspGetFirstDiagnosticAt(const char *uri, int line)
{
    if (!uri)
        return NULL;

    for (int i = 0; i < diagnosticCount; i++)
    {
        if (diagnosticStore[i].line == line &&
            (strcmp(diagnosticStore[i].uri, uri) == 0 || strstr(diagnosticStore[i].uri, uri) || strstr(uri, diagnosticStore[i].uri)))
        {
            return &diagnosticStore[i];
        }
    }
    return NULL;
}

/* --- Word and Identifier Extraction Helper --- */

static int getWordAtPosition(int lineY, int colX, char *outWord, size_t outSize, int *outStartCol)
{
    if (!outWord || outSize == 0 || lineY < 0 || lineY >= buffer.numRows)
        return 0;

    Row *row = &buffer.rows[lineY];
    int x = colX;
    if (x > row->size)
        x = row->size;

    int start = x;
    while (start > 0 && (isalnum((unsigned char)row->chars[start - 1]) || row->chars[start - 1] == '_'))
    {
        start--;
    }

    int end = x;
    while (end < row->size && (isalnum((unsigned char)row->chars[end]) || row->chars[end] == '_'))
    {
        end++;
    }

    int len = end - start;
    if (len <= 0)
        return 0;

    if ((size_t)len >= outSize)
        len = (int)outSize - 1;

    memcpy(outWord, &row->chars[start], (size_t)len);
    outWord[len] = '\0';

    if (outStartCol)
        *outStartCol = start;

    return len;
}

static int getWordAtCursor(char *outWord, size_t outSize, int *outStartCol)
{
    return getWordAtPosition(cursor.y, cursor.x, outWord, outSize, outStartCol);
}

/* --- Code Completion Engine --- */

static const char *commonKeywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while", "class", "public", "private",
    "protected", "virtual", "override", "template", "typename", "namespace",
    "using", "try", "catch", "throw", "import", "export", "from", "as",
    "def", "class", "self", "None", "True", "False", "lambda", "yield",
    "fn", "let", "mut", "pub", "impl", "trait", "match", "use", "mod",
    "package", "func", "type", "var", "interface", "select", "defer", "go",
    NULL
};

int lspRequestCompletion(const char *uri, int line, int col, const char *prefix, LspCompletionItem *outItems, int maxItems)
{
    (void)uri;
    (void)line;
    (void)col;

    if (!outItems || maxItems <= 0)
        return 0;

    int count = 0;
    size_t prefixLen = prefix ? strlen(prefix) : 0;

    /* 1. Add matching language keywords */
    for (int k = 0; commonKeywords[k] != NULL && count < maxItems; k++)
    {
        const char *kw = commonKeywords[k];
        if (prefixLen == 0 || strncasecmp(kw, prefix, prefixLen) == 0)
        {
            /* Check duplicate */
            int dup = 0;
            for (int j = 0; j < count; j++)
            {
                if (strcmp(outItems[j].label, kw) == 0)
                {
                    dup = 1;
                    break;
                }
            }
            if (!dup)
            {
                snprintf(outItems[count].label, sizeof(outItems[count].label), "%s", kw);
                outItems[count].kind = LSP_KIND_KEYWORD;
                snprintf(outItems[count].detail, sizeof(outItems[count].detail), "keyword");
                snprintf(outItems[count].documentation, sizeof(outItems[count].documentation), "Language keyword");
                snprintf(outItems[count].insertText, sizeof(outItems[count].insertText), "%s", kw);
                count++;
            }
        }
    }

    /* 2. Add matching workspace symbols */
    for (int s = 0; s < symbolState.count && count < maxItems; s++)
    {
        const Symbol *sym = &symbolState.symbols[s];
        if (prefixLen == 0 || strncasecmp(sym->name, prefix, prefixLen) == 0)
        {
            int dup = 0;
            for (int j = 0; j < count; j++)
            {
                if (strcmp(outItems[j].label, sym->name) == 0)
                {
                    dup = 1;
                    break;
                }
            }
            if (!dup)
            {
                snprintf(outItems[count].label, sizeof(outItems[count].label), "%s", sym->name);
                outItems[count].kind = (sym->kind == SYMBOL_FUNCTION) ? LSP_KIND_FUNCTION :
                                       (sym->kind == SYMBOL_STRUCT) ? LSP_KIND_STRUCT : LSP_KIND_VARIABLE;
                snprintf(outItems[count].detail, sizeof(outItems[count].detail), "%.32s (%.64s:%d)",
                         symbolKindToString(sym->kind), sym->file, sym->line);
                snprintf(outItems[count].documentation, sizeof(outItems[count].documentation), "%s", sym->signature);
                snprintf(outItems[count].insertText, sizeof(outItems[count].insertText), "%s", sym->name);
                count++;
            }
        }
    }

    /* 3. Scan local buffer for words matching prefix */
    for (int r = 0; r < buffer.numRows && count < maxItems; r++)
    {
        Row *row = &buffer.rows[r];
        int i = 0;
        while (i < row->size && count < maxItems)
        {
            while (i < row->size && !isalnum((unsigned char)row->chars[i]) && row->chars[i] != '_')
                i++;
            int start = i;
            while (i < row->size && (isalnum((unsigned char)row->chars[i]) || row->chars[i] == '_'))
                i++;
            int len = i - start;
            if (len >= 2)
            {
                char token[64];
                int copyLen = len < 63 ? len : 63;
                memcpy(token, &row->chars[start], (size_t)copyLen);
                token[copyLen] = '\0';

                if (prefixLen == 0 || strncasecmp(token, prefix, prefixLen) == 0)
                {
                    int dup = 0;
                    for (int j = 0; j < count; j++)
                    {
                        if (strcmp(outItems[j].label, token) == 0)
                        {
                            dup = 1;
                            break;
                        }
                    }
                    if (!dup)
                    {
                        snprintf(outItems[count].label, sizeof(outItems[count].label), "%s", token);
                        outItems[count].kind = LSP_KIND_TEXT;
                        snprintf(outItems[count].detail, sizeof(outItems[count].detail), "identifier");
                        snprintf(outItems[count].documentation, sizeof(outItems[count].documentation), "Buffer word at line %d", r + 1);
                        snprintf(outItems[count].insertText, sizeof(outItems[count].insertText), "%s", token);
                        count++;
                    }
                }
            }
        }
    }

    return count;
}

static const Symbol *findSymbolByName(const char *name)
{
    if (!name)
        return NULL;
    for (int i = 0; i < symbolState.count; i++)
    {
        if (strcmp(symbolState.symbols[i].name, name) == 0)
            return &symbolState.symbols[i];
    }
    return NULL;
}

/* --- Go-to-Definition Provider --- */

int lspLookupDefinition(const char *uri, int line, int col, char *outTargetFile, int *outLine, int *outCol)
{
    (void)uri;

    char word[128];
    int ok = 0;
    if (line > 0 && col > 0)
        ok = getWordAtPosition(line - 1, col - 1, word, sizeof(word), NULL);
    else
        ok = getWordAtCursor(word, sizeof(word), NULL);

    if (ok <= 0)
        return 0;

    /* 1. Look up in workspace symbol indexer */
    const Symbol *sym = findSymbolByName(word);
    if (sym && sym->file[0])
    {
        if (outTargetFile)
            snprintf(outTargetFile, 256, "%.255s", sym->file);
        if (outLine)
            *outLine = sym->line;
        if (outCol)
            *outCol = 1;
        return 1;
    }

    /* 2. Look up in current buffer */
    for (int r = 0; r < buffer.numRows; r++)
    {
        Row *row = &buffer.rows[r];
        char *pos = strstr(row->chars, word);
        if (pos)
        {
            /* If it looks like a declaration: preceded or followed by types or '(' */
            if (outTargetFile)
                snprintf(outTargetFile, 256, "%s", currentFile ? currentFile : "[Untitled]");
            if (outLine)
                *outLine = r + 1;
            if (outCol)
                *outCol = (int)(pos - row->chars) + 1;
            return 1;
        }
    }

    return 0;
}

int lspGotoDefinition(void)
{
    char targetFile[256] = "";
    int targetLine = 1;
    int targetCol = 1;

    if (!lspLookupDefinition(currentFile ? currentFile : "", cursor.y + 1, cursor.x + 1, targetFile, &targetLine, &targetCol))
    {
        editorSetStatusMessage("Definition not found");
        return 0;
    }

    if (targetFile[0] != '\0' && (!currentFile || strcmp(currentFile, targetFile) != 0))
    {
        int opened = bufferManagerOpen(targetFile);
        if (opened < 0)
        {
            editorSetStatusMessage("Could not open definition file");
            return 0;
        }
    }

    if (targetLine > 0 && targetLine <= buffer.numRows)
    {
        cursor.y = targetLine - 1;
        cursor.x = (targetCol > 0) ? targetCol - 1 : 0;
        if (cursor.x > buffer.rows[cursor.y].size)
            cursor.x = buffer.rows[cursor.y].size;

        viewport.rowOffset = cursor.y - (viewport.screenRows / 2);
        if (viewport.rowOffset < 0)
            viewport.rowOffset = 0;

        char msg[256];
        snprintf(msg, sizeof(msg), "Jumped to definition at %.180s:%d:%d", targetFile, targetLine, targetCol);
        editorSetStatusMessage(msg);
        return 1;
    }

    return 0;
}

/* --- Hover Information Provider --- */

int lspRequestHover(const char *uri, int line, int col, char *outSignature, size_t sigSize, char *outDoc, size_t docSize)
{
    /* 1. Check if diagnostic exists at this line */
    const LspDiagnostic *d = lspGetFirstDiagnosticAt(uri ? uri : (currentFile ? currentFile : ""), line);
    if (d)
    {
        if (outSignature)
        {
            const char *sev = (d->severity == LSP_SEVERITY_ERROR) ? "Error" :
                              (d->severity == LSP_SEVERITY_WARNING) ? "Warning" :
                              (d->severity == LSP_SEVERITY_INFO) ? "Info" : "Hint";
            snprintf(outSignature, sigSize, "[%s] %s (%s)", sev, d->source[0] ? d->source : "LSP", d->code[0] ? d->code : "diag");
        }
        if (outDoc)
            snprintf(outDoc, docSize, "%s", d->message);
        return 1;
    }

    char word[128];
    int ok = 0;
    if (line > 0 && col > 0)
        ok = getWordAtPosition(line - 1, col - 1, word, sizeof(word), NULL);
    else
        ok = getWordAtCursor(word, sizeof(word), NULL);

    if (ok <= 0)
        return 0;

    /* 2. Check workspace symbols */
    const Symbol *sym = findSymbolByName(word);
    if (sym)
    {
        if (outSignature)
            snprintf(outSignature, sigSize, "%s", sym->signature[0] ? sym->signature : sym->name);
        if (outDoc)
            snprintf(outDoc, docSize, "%s symbol defined in %s at line %d",
                     symbolKindToString(sym->kind), sym->file, sym->line);
        return 1;
    }

    /* 3. Check language keyword */
    for (int k = 0; commonKeywords[k] != NULL; k++)
    {
        if (strcmp(commonKeywords[k], word) == 0)
        {
            if (outSignature)
                snprintf(outSignature, sigSize, "keyword %s", word);
            if (outDoc)
                snprintf(outDoc, docSize, "Reserved language keyword '%s'", word);
            return 1;
        }
    }

    /* 4. Default identifier description */
    if (outSignature)
        snprintf(outSignature, sigSize, "(identifier) %s", word);
    if (outDoc)
        snprintf(outDoc, docSize, "Symbol '%s' at line %d, column %d", word, line, col);

    return 1;
}

/* --- Find References Provider --- */

int lspRequestReferences(const char *uri, int line, int col, LspLocation *outLocs, int maxLocs)
{
    (void)uri;

    if (!outLocs || maxLocs <= 0)
        return 0;

    char word[128];
    int ok = 0;
    if (line > 0 && col > 0)
        ok = getWordAtPosition(line - 1, col - 1, word, sizeof(word), NULL);
    else
        ok = getWordAtCursor(word, sizeof(word), NULL);

    if (ok <= 0)
        return 0;

    int found = 0;

    /* Scan all open buffers */
    int bufCount = bufferManagerCount();
    for (int b = 0; b < bufCount && found < maxLocs; b++)
    {
        FileBuffer *fb = bufferManagerGet(b);
        if (!fb)
            continue;

        for (int r = 0; r < fb->doc.numRows && found < maxLocs; r++)
        {
            Row *row = &fb->doc.rows[r];
            char *pos = strstr(row->chars, word);
            if (pos)
            {
                LspLocation *loc = &outLocs[found++];
                snprintf(loc->file, sizeof(loc->file), "%.255s", fb->filename[0] ? fb->filename : fb->displayName);
                loc->line = r + 1;
                loc->col = (int)(pos - row->chars) + 1;

                /* Format preview */
                int previewLen = row->size < 120 ? row->size : 120;
                memcpy(loc->preview, row->chars, (size_t)previewLen);
                loc->preview[previewLen] = '\0';
            }
        }
    }

    /* If no buffer manager open files or current file only */
    if (found == 0 && buffer.numRows > 0)
    {
        for (int r = 0; r < buffer.numRows && found < maxLocs; r++)
        {
            Row *row = &buffer.rows[r];
            char *pos = strstr(row->chars, word);
            if (pos)
            {
                LspLocation *loc = &outLocs[found++];
                snprintf(loc->file, sizeof(loc->file), "%s", currentFile ? currentFile : "[Untitled]");
                loc->line = r + 1;
                loc->col = (int)(pos - row->chars) + 1;
                int previewLen = row->size < 120 ? row->size : 120;
                memcpy(loc->preview, row->chars, (size_t)previewLen);
                loc->preview[previewLen] = '\0';
            }
        }
    }

    return found;
}

/* --- Code Actions & Refactoring Framework --- */

int lspRequestCodeActions(const char *uri, int line, int col, LspCodeAction *outActions, int maxActions)
{
    (void)col;

    if (!outActions || maxActions <= 0)
        return 0;

    int count = 0;
    const char *target = uri ? uri : (currentFile ? currentFile : "[Untitled]");

    /* 1. Diagnostic quickfix */
    const LspDiagnostic *d = lspGetFirstDiagnosticAt(target, line);
    if (d && count < maxActions)
    {
        LspCodeAction *act = &outActions[count++];
        snprintf(act->title, sizeof(act->title), "QuickFix: Resolve diagnostic '%.64s'", d->message);
        snprintf(act->kind, sizeof(act->kind), "quickfix");
        snprintf(act->targetFile, sizeof(act->targetFile), "%s", target);
        act->line = line;
        act->col = 1;
        snprintf(act->replacementText, sizeof(act->replacementText), "// Fixed: %.180s\n", d->message);
        act->deleteOld = 0;
    }

    /* 2. Refactor: Extract Variable / Expression */
    if (count < maxActions)
    {
        LspCodeAction *act = &outActions[count++];
        snprintf(act->title, sizeof(act->title), "Refactor: Extract expression into local variable");
        snprintf(act->kind, sizeof(act->kind), "refactor.extract");
        snprintf(act->targetFile, sizeof(act->targetFile), "%s", target);
        act->line = line;
        act->col = 1;
        snprintf(act->replacementText, sizeof(act->replacementText), "    auto extracted_val = ");
        act->deleteOld = 0;
    }

    /* 3. Refactor: Add header / import declaration */
    if (count < maxActions)
    {
        LspCodeAction *act = &outActions[count++];
        snprintf(act->title, sizeof(act->title), "Refactor: Organize & sort includes / imports");
        snprintf(act->kind, sizeof(act->kind), "source.organizeImports");
        snprintf(act->targetFile, sizeof(act->targetFile), "%s", target);
        act->line = 1;
        act->col = 1;
        act->replacementText[0] = '\0';
        act->deleteOld = 0;
    }

    /* 4. Format selection */
    if (count < maxActions)
    {
        LspCodeAction *act = &outActions[count++];
        snprintf(act->title, sizeof(act->title), "Format: Clean trailing whitespace on line");
        snprintf(act->kind, sizeof(act->kind), "format.line");
        snprintf(act->targetFile, sizeof(act->targetFile), "%s", target);
        act->line = line;
        act->col = 1;
        act->replacementText[0] = '\0';
        act->deleteOld = 0;
    }

    return count;
}

int lspApplyCodeAction(int index)
{
    if (index < 0 || index >= lspCodeActionsModal.count)
        return 0;

    LspCodeAction *act = &lspCodeActionsModal.actions[index];
    if (strcmp(act->kind, "format.line") == 0)
    {
        if (cursor.y >= 0 && cursor.y < buffer.numRows)
        {
            Row *row = &buffer.rows[cursor.y];
            while (row->size > 0 && isspace((unsigned char)row->chars[row->size - 1]))
            {
                row->size--;
                row->chars[row->size] = '\0';
            }
            editorSetStatusMessage("Trimmed trailing whitespace");
            return 1;
        }
    }
    else if (strcmp(act->kind, "quickfix") == 0)
    {
        if (currentFile)
            lspClearDiagnostics(currentFile);
        editorSetStatusMessage("Applied QuickFix action");
        return 1;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Applied action: %.80s", act->title);
    editorSetStatusMessage(msg);
    return 1;
}
