#include <string.h>
#include <stdio.h>
#include "plugin_api.h"
#include "plugin_commands.h"
#include "plugin.h"
#include "editor.h"
#include "buffer.h"
#include <time.h>

extern Buffer buffer;
extern GitViewState gitView;
extern PluginManagerState pluginManager;

static void pluginLog(
    const char *message
)
{
    FILE *fp =
        fopen(
            "logs/plugins.log",
            "a"
        );

    if (!fp)
        return;

    time_t now =
        time(NULL);

    struct tm *tmInfo =
        localtime(&now);

    char timestamp[64];

    strftime(
        timestamp,
        sizeof(timestamp),
        "%Y-%m-%d %H:%M:%S",
        tmInfo
    );

    fprintf(
        fp,
        "[%s] %s\n",
        timestamp,
        message
    );

    fclose(fp);
}

static void pluginError(
    Plugin *plugin,
    const char *message
)
{
    char logMsg[256];

    snprintf(
        logMsg,
        sizeof(logMsg),
        "[ERROR] %s",
        message
    );

    pluginLog(logMsg);

    editorSetStatusMessage(
        logMsg
    );

    if (plugin)
    {
        plugin->errorCount++;
    }
}

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
    Plugin *plugin =
        pluginFindByCommand(
            command
        );

    if (
        plugin &&
        !plugin->enabled
    )
    {
        char msg[256];

snprintf(
    msg,
    sizeof(msg),
    "Plugin disabled: %s",
    plugin->name
);

pluginError(
    plugin,
    msg
);

return;
    }

    PluginCommandFunc func =
        pluginFindCommand(
            command
        );

    if (func)
{
    for (int i = 0; i < pluginCountValue; i++)
    {
        if (strcmp(
                plugins[i].command,
                command
            ) == 0)
        {
            plugins[i].commandRuns++;
            break;
        }
    }

    func();
    return;
}

    char msg[256];

snprintf(
    msg,
    sizeof(msg),
    "Unknown plugin command: %s",
    command
);

pluginError(
    NULL,
    msg
);
}
static void startupHook(void)
{
    Plugin *p =
        pluginFindByCommand("hello");

    if (p)
        p->startupHooks++;

    pluginLog(
        "[STARTUP] Hello Plugin saw startup"
    );
}

static void searchHook(void)
{
    Plugin *p =
        pluginFindByCommand("hello");

    if (p)
        p->searchHooks++;

    pluginLog(
        "[SEARCH] Hello Plugin saw search"
    );
}
 
static void saveHook(void)
{
    Plugin *p =
        pluginFindByCommand("hello");

    if (p)
        p->saveHooks++;

    pluginLog(
        "[SAVE] Hello Plugin saw save"
    );
}

static void exitHook(void)
{
    Plugin *p =
        pluginFindByCommand("hello");

    if (p)
        p->exitHooks++;

    pluginLog(
        "[EXIT] Hello Plugin saw exit"
    );
}

void pluginShowInfo(void)
{
    static int current = 0;

    if (pluginCountValue == 0)
    {
        editorSetStatusMessage(
            "No plugins loaded"
        );
        return;
    }

    Plugin *p = &plugins[current];

    char message[256];

    snprintf(
    message,
    sizeof(message),
    "%s v%s [%s] Auto:%s Events:%s",
    p->name,
    p->version,
    p->enabled ? "Enabled" : "Disabled",
    p->autoLoad ? "On" : "Off",
    p->allowEvents ? "On" : "Off"
);

    editorSetStatusMessage(
        message
    );

    current++;

    if (current >= pluginCountValue)
        current = 0;
}

void pluginOpenLog(void)
{
    openGitView(
        "logs/plugins.log"
    );

    gitView.pluginLog = 1;

    editorSetStatusMessage(
        "Plugin Log (Esc to return)"
    );
}

void pluginClearLogs(void)
{
    FILE *fp =
        fopen(
            "logs/plugins.log",
            "w"
        );

    if (!fp)
    {
        editorSetStatusMessage(
            "Could not clear plugin log"
        );
        return;
    }

    fclose(fp);

    editorSetStatusMessage(
        "Plugin log cleared"
    );
}

void pluginRegisterBuiltins(void)
{
    pluginRegisterCommand(
        "hello",
        helloPlugin
    );

    pluginRegisterCommand(
        "stats",
        statsPlugin
    );

    pluginRegisterCommand(
        "format",
        formatterPlugin
    );

    pluginRegisterEvent(
        PLUGIN_EVENT_STARTUP,
        startupHook
    );

    pluginRegisterEvent(
        PLUGIN_EVENT_SEARCH,
        searchHook
    );

    pluginRegisterEvent(
        PLUGIN_EVENT_SAVE,
        saveHook
    );

    pluginRegisterEvent(
        PLUGIN_EVENT_EXIT,
        exitHook
    );
}
void pluginShowStats(void)
{
    Plugin *p =
        pluginGet(
            pluginManager.selected
        );

    if (!p)
    {
        editorSetStatusMessage(
            "No plugin selected"
        );
        return;
    }

    char msg[256];

    snprintf(
        msg,
        sizeof(msg),
       "%s | Cmd:%d Err:%d Startup:%d Save:%d Search:%d Exit:%d",
        p->name,
        p->commandRuns,
        p->errorCount,
        p->startupHooks,
        p->saveHooks,
        p->searchHooks,
        p->exitHooks
    );

    editorSetStatusMessage(msg);
}
void pluginShowCommands(void)
{
    char msg[256];

    snprintf(
        msg,
        sizeof(msg),
        "Commands: hello stats format"
    );

    editorSetStatusMessage(
        msg
    );
}
void pluginToggle(void)
{
    Plugin *p =
        pluginGet(
            pluginManager.selected
        );

    if (!p)
    {
        editorSetStatusMessage(
            "No plugin selected"
        );
        return;
    }

    p->enabled =
        !p->enabled;

    char msg[128];

    snprintf(
        msg,
        sizeof(msg),
        "%s %s",
        p->name,
        p->enabled
            ? "enabled"
            : "disabled"
    );

    editorSetStatusMessage(msg);
}