#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "plugin.h"

Plugin plugins[MAX_PLUGINS];
int pluginCountValue = 0;

void pluginInit(void)
{
    pluginCountValue = 0;
}

static void trim(char *s)
{
    size_t len = strlen(s);

    while (len > 0 &&
          (s[len - 1] == '\n' ||
           s[len - 1] == '\r'))
    {
        s[--len] = '\0';
    }
}

int pluginLoad(const char *path)
{
    if (pluginCountValue >= MAX_PLUGINS)
        return 0;

    FILE *fp = fopen(path, "r");

    if (!fp)
        return 0;

    Plugin p;
    memset(&p, 0, sizeof(p));

    strcpy(p.filename, path);

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        trim(line);

        char *eq = strchr(line, '=');

        if (!eq)
            continue;

        *eq = '\0';

        char *key = line;
        char *value = eq + 1;

        if (strcmp(key, "name") == 0)
            strncpy(p.name,
                    value,
                    sizeof(p.name) - 1);

        else if (strcmp(key, "version") == 0)
            strncpy(p.version,
                    value,
                    sizeof(p.version) - 1);

        else if (strcmp(key, "author") == 0)
            strncpy(p.author,
                    value,
                    sizeof(p.author) - 1);

        else if (strcmp(key, "command") == 0)
            strncpy(p.command,
                    value,
                    sizeof(p.command) - 1);
    }

    fclose(fp);

    plugins[pluginCountValue++] = p;

    return 1;
}

void pluginLoadAll(void)
{
    DIR *dir = opendir("plugins");

    if (!dir)
        return;

    struct dirent *entry;

    while ((entry = readdir(dir)))
    {
        if (strstr(entry->d_name, ".plugin"))
        {
            char path[512];

            snprintf(path,
                     sizeof(path),
                     "plugins/%s",
                     entry->d_name);

            pluginLoad(path);
        }
    }

    closedir(dir);
}

void pluginReloadAll(void)
{
    pluginCountValue = 0;

    pluginLoadAll();
}

int pluginCount(void)
{
    return pluginCountValue;
}

Plugin *pluginGet(int index)
{
    if (index < 0 ||
        index >= pluginCountValue)
    {
        return NULL;
    }

    return &plugins[index];
}