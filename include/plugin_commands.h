#ifndef PLUGIN_COMMANDS_H
#define PLUGIN_COMMANDS_H

void pluginExecute(
    const char *command
);

void pluginRegisterBuiltins(void);

void pluginShowInfo(void);

void pluginOpenLog(void);

void pluginShowStats(void);

void pluginClearLogs(void);

void pluginShowCommands(void);

void pluginToggle(void);

#endif