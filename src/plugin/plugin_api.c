#include <string.h>
#include <stdio.h>
#include "plugin_api.h"

static PluginCommand commands[MAX_PLUGIN_COMMANDS];
static int commandCount = 0;

static PluginEventHandler handlers[MAX_PLUGIN_EVENTS];
static int handlerCount = 0;

void pluginApiInit(void)
{
    commandCount = 0;
    handlerCount = 0;
}

int pluginRegisterCommand(
    const char *command,
    PluginCommandFunc func
)
{
    if (commandCount >= MAX_PLUGIN_COMMANDS)
        return 0;

    strcpy(
        commands[commandCount].command,
        command
    );

    commands[commandCount].execute =
        func;

    commandCount++;

    return 1;
}

PluginCommandFunc pluginFindCommand(
    const char *command
)
{
    for (int i = 0;
         i < commandCount;
         i++)
    {
        if (strcmp(
                commands[i].command,
                command
            ) == 0)
        {
            return commands[i].execute;
        }
    }

    return NULL;
}

int pluginRegisterEvent(
    PluginEvent event,
    PluginCommandFunc func
)
{
    if (handlerCount >= MAX_PLUGIN_EVENTS)
        return 0;

    handlers[handlerCount].event =
        event;

    handlers[handlerCount].callback =
        func;

    handlerCount++;

    return 1;
}

void pluginTriggerEvent(
    PluginEvent event
)
{
    for (int i = 0;
         i < handlerCount;
         i++)
    {
        if (handlers[i].event == event)
        {
            handlers[i].callback();
        }
    }
}