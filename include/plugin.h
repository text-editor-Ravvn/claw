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
} Plugin;

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

/* Phase 1/2 editor integration */
void pluginList(void);

void pluginReload(void);

#endif