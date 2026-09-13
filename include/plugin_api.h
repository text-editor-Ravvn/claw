#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include "plugin.h"

#define MAX_PLUGIN_COMMANDS 64
#define MAX_PLUGIN_EVENTS 64

typedef void (*PluginCommandFunc)(void);

typedef struct
{
    char command[64];
    PluginCommandFunc execute;
} PluginCommand;

typedef struct
{
    PluginEvent event;
    PluginCommandFunc callback;
} PluginEventHandler;

void pluginApiInit(void);

int pluginRegisterCommand(
    const char *command,
    PluginCommandFunc func
);

PluginCommandFunc pluginFindCommand(
    const char *command
);

int pluginRegisterEvent(
    PluginEvent event,
    PluginCommandFunc func
);

void pluginTriggerEvent(
    PluginEvent event
);

#endif