#ifndef PLUGIN_H
#define PLUGIN_H

#define MAX_PLUGINS 64

typedef struct
{
    char name[64];
    char version[32];
    char author[64];
    char command[64];

    char filename[256];

    int enabled;

    /* Existing statistics */
    int loadCount;
    int commandCount;
    int eventCount;

    /* V1.5.3 statistics */
    int commandRuns;
    int errorCount;
    int startupHooks;
    int saveHooks;
    int searchHooks;
    int exitHooks;

} Plugin;

typedef enum
{
    PLUGIN_EVENT_STARTUP,
    PLUGIN_EVENT_SAVE,
    PLUGIN_EVENT_SEARCH,
    PLUGIN_EVENT_EXIT
} PluginEvent;

typedef struct
{
    int active;
    int selected;
} PluginManagerState;

extern Plugin plugins[MAX_PLUGINS];

extern int pluginCountValue;

void pluginInit(void);

int pluginLoad(
    const char *path
);

void pluginLoadAll(void);

void pluginReloadAll(void);

int pluginCount(void);

Plugin *pluginGet(int index);

Plugin *pluginFindByCommand(
    const char *command
);

void openPluginManager(void);
void closePluginManager(void);

#endif