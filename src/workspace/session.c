#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "session.h"
#include "workspace.h"
#include "editor.h"
#include "cursor.h"
#include "viewport.h"
#include "fileio.h"
#include "buffer.h"
#include "recent.h"
#include "utils.h"

EditorSession currentSession = {0};
extern char *currentFile;

static void getDefaultSessionPath(char *outPath, size_t outSize)
{
    if (workspaceIsActive())
    {
        const char *root = workspaceGetRoot();
        if (root)
        {
            char clawDir[SESSION_PATH_MAX];
            snprintf(clawDir, sizeof(clawDir), "%s/.claw", root);
#ifdef _WIN32
            mkdir(clawDir);
#else
            mkdir(clawDir, 0755);
#endif
            snprintf(outPath, outSize, "%s/.claw/session.claw", root);
            return;
        }
    }

    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg && strlen(xdg) > 0)
    {
        snprintf(outPath, outSize, "%s/claw/session.claw", xdg);
        return;
    }

    const char *home = getenv("HOME");
    if (home && strlen(home) > 0)
    {
        snprintf(outPath, outSize, "%s/.config/claw/session.claw", home);
        return;
    }

    snprintf(outPath, outSize, "config/session.claw");
}

int sessionSave(const char *path)
{
    if (!path || !*path)
        return 0;

    FILE *fp = fopen(path, "w");
    if (!fp)
        return 0;

    const char *root = workspaceGetRoot();
    fprintf(fp, "# Claw Editor Session\n");
    fprintf(fp, "workspace=%s\n", root ? root : "");
    fprintf(fp, "file=%s\n", currentFile ? currentFile : "");
    fprintf(fp, "cursor_x=%d\n", cursor.x);
    fprintf(fp, "cursor_y=%d\n", cursor.y);
    fprintf(fp, "row_offset=%d\n", viewport.rowOffset);
    fprintf(fp, "col_offset=%d\n", viewport.colOffset);

    fclose(fp);
    return 1;
}

int sessionRestore(const char *path)
{
    if (!path || !*path)
        return 0;

    FILE *fp = fopen(path, "r");
    if (!fp)
        return 0;

    char line[SESSION_PATH_MAX];
    char wsPath[SESSION_PATH_MAX] = "";
    char filePath[SESSION_PATH_MAX] = "";
    int curX = 0, curY = 0, rowOff = 0, colOff = 0;

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\r\n")] = '\0';
        char *trimmed = trimWhitespace(line);
        if (*trimmed == '\0' || *trimmed == '#')
            continue;

        char *equals = strchr(trimmed, '=');
        if (!equals)
            continue;

        *equals = '\0';
        char *key = trimWhitespace(trimmed);
        char *val = trimWhitespace(equals + 1);

        if (strcmp(key, "workspace") == 0)
        {
            snprintf(wsPath, sizeof(wsPath), "%s", val);
        }
        else if (strcmp(key, "file") == 0)
        {
            snprintf(filePath, sizeof(filePath), "%s", val);
        }
        else if (strcmp(key, "cursor_x") == 0)
        {
            curX = atoi(val);
        }
        else if (strcmp(key, "cursor_y") == 0)
        {
            curY = atoi(val);
        }
        else if (strcmp(key, "row_offset") == 0)
        {
            rowOff = atoi(val);
        }
        else if (strcmp(key, "col_offset") == 0)
        {
            colOff = atoi(val);
        }
    }

    fclose(fp);

    if (strlen(wsPath) > 0)
    {
        workspaceOpen(wsPath);
    }

    if (strlen(filePath) > 0)
    {
        if (openFile(filePath))
        {
            if (currentFile)
                free(currentFile);
            currentFile = strdup(filePath);

            if (curY >= buffer.numRows)
                curY = buffer.numRows - 1;
            if (curY < 0)
                curY = 0;
            cursor.y = curY;

            int rowLen = (curY < buffer.numRows) ? buffer.rows[curY].size : 0;
            if (curX > rowLen)
                curX = rowLen;
            if (curX < 0)
                curX = 0;
            cursor.x = curX;

            viewport.rowOffset = rowOff;
            viewport.colOffset = colOff;

            recentAddFile(filePath);
            return 1;
        }
    }

    return 0;
}

int sessionAutoSave(void)
{
    char defPath[SESSION_PATH_MAX];
    getDefaultSessionPath(defPath, sizeof(defPath));
    return sessionSave(defPath);
}

int sessionAutoRestore(void)
{
    char defPath[SESSION_PATH_MAX];
    getDefaultSessionPath(defPath, sizeof(defPath));
    return sessionRestore(defPath);
}

void sessionInit(void)
{
    memset(&currentSession, 0, sizeof(currentSession));
}
