#include <string.h>
#include <stdio.h>

#include "plugin_commands.h"
#include "plugin.h"
#include "editor.h"
#include "buffer.h"

extern Buffer buffer;

static void helloPlugin(void)
{
    editorSetStatusMessage(
        "Hello from Hello Plugin"
    );
}

static void statsPlugin(void)
{
    int words = 0;
    int chars = 0;

    for (int i = 0; i < buffer.numRows; i++)
    {
        chars += buffer.rows[i].size;

        int inWord = 0;

        for (int j = 0;
             j < buffer.rows[i].size;
             j++)
        {
            char c =
                buffer.rows[i].chars[j];

            if (c == ' ' ||
                c == '\t')
            {
                inWord = 0;
            }
            else if (!inWord)
            {
                words++;
                inWord = 1;
            }
        }
    }

    char msg[128];

    snprintf(
        msg,
        sizeof(msg),
        "Lines:%d Words:%d Chars:%d",
        buffer.numRows,
        words,
        chars
    );

    editorSetStatusMessage(msg);
}

static void formatterPlugin(void)
{
    editorSetStatusMessage(
        "Document formatted"
    );
}

void pluginExecute(
    const char *command
)
{
    if (strcmp(command, "hello") == 0)
    {
        helloPlugin();
        return;
    }

    if (strcmp(command, "stats") == 0)
    {
        statsPlugin();
        return;
    }

    if (strcmp(command, "format") == 0)
    {
        formatterPlugin();
        return;
    }

    editorSetStatusMessage(
        "Unknown plugin"
    );
}