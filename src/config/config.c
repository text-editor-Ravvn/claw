#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "config.h"
#include "utils.h"

EditorConfig editorConfig;
static char userConfigPath[512] = "";

void configInit(void) {
  /* Compiled-in defaults matching a reasonable editing experience. */
  editorConfig.tabWidth = 4;
  editorConfig.showLineNumbers = 0;
  editorConfig.showWelcome = 1;
  editorConfig.showStatusBar = 1;
  editorConfig.statusbarShowVersion = -1;
  editorConfig.statusbarShowWorkspace = -1;
  editorConfig.statusbarShowFilename = -1;
  editorConfig.statusbarShowLanguage = -1;
  editorConfig.statusbarShowGit = -1;
  editorConfig.statusbarShowBookmarks = -1;
  editorConfig.statusbarShowTheme = -1;
  editorConfig.statusbarShowLineCount = -1;
  snprintf(editorConfig.keymapName, sizeof(editorConfig.keymapName), "default");
}

void configSetUserPath(const char *path)
{
  if(!path)
  {
    userConfigPath[0] = '\0';
    return;
  }

  snprintf(
    userConfigPath,
    sizeof(userConfigPath),
    "%s",
    path
  );
}

int configLoad(const char *path) {
  FILE *fp = fopen(path, "r");

  if (!fp)
    return 0;

  char line[256];

  while (fgets(line, sizeof(line), fp)) {
    /* Strip the trailing newline. */
    line[strcspn(line, "\r\n")] = '\0';

    char *trimmed = trimWhitespace(line);

    /* Skip blank lines and comments. */
    if (*trimmed == '\0' || *trimmed == '#')
      continue;

    /* Split on the first '=' sign. */
    char *equals = strchr(trimmed, '=');

    if (!equals)
      continue;

    *equals = '\0';

    char *key = trimWhitespace(trimmed);
    char *value = trimWhitespace(equals + 1);

    if (strcmp(key, "tab_width") == 0) {
      int width = atoi(value);

      if (width >= 1 && width <= 16)
        editorConfig.tabWidth = width;
    } else if (strcmp(key, "show_line_numbers") == 0) {
      editorConfig.showLineNumbers = parseBool(value);
    } else if (strcmp(key, "show_welcome") == 0) {
      editorConfig.showWelcome = parseBool(value);
    } else if (strcmp(key, "show_statusbar") == 0) {
      editorConfig.showStatusBar = parseBool(value);
    } else if (strcmp(key, "statusbar_show_version") == 0) {
      editorConfig.statusbarShowVersion = parseBool(value);
    } else if (strcmp(key, "statusbar_show_workspace") == 0) {
      editorConfig.statusbarShowWorkspace = parseBool(value);
    } else if (strcmp(key, "statusbar_show_filename") == 0) {
      editorConfig.statusbarShowFilename = parseBool(value);
    } else if (strcmp(key, "statusbar_show_language") == 0) {
      editorConfig.statusbarShowLanguage = parseBool(value);
    } else if (strcmp(key, "statusbar_show_git") == 0) {
      editorConfig.statusbarShowGit = parseBool(value);
    } else if (strcmp(key, "statusbar_show_bookmarks") == 0) {
      editorConfig.statusbarShowBookmarks = parseBool(value);
    } else if (strcmp(key, "statusbar_show_theme") == 0) {
      editorConfig.statusbarShowTheme = parseBool(value);
    } else if (strcmp(key, "statusbar_show_line_count") == 0) {
      editorConfig.statusbarShowLineCount = parseBool(value);
    } else if (strcmp(key, "keymap") == 0) {
      if (strlen(value) > 0 &&
          strlen(value) < sizeof(editorConfig.keymapName)) {
        snprintf(editorConfig.keymapName, sizeof(editorConfig.keymapName), "%s",
                 value);
      }
    }
    /* Unknown keys are silently ignored. */
  }

  fclose(fp);
  return 1;
}


static const char *statusbarKeys[] = {
    "statusbar_show_version",
    "statusbar_show_workspace",
    "statusbar_show_filename",
    "statusbar_show_language",
    "statusbar_show_git",
    "statusbar_show_bookmarks",
    "statusbar_show_theme",
    "statusbar_show_line_count"
};

int configSaveStatusBarSettings(const int values[], int count)
{
    if (!values || count != 8 || userConfigPath[0] == '\0')
        return 0;

    for (int i = 0; i < count; i++)
    {
        if (values[i] != 0 && values[i] != 1)
            return 0;
    }

    char directory[512];
    snprintf(directory, sizeof(directory), "%s", userConfigPath);

    char *slash = strrchr(directory, '/');
    if (!slash)
        return 0;

    *slash = '\0';

    /* Create missing parent directories. */
    for (char *p = directory + 1; ; p++)
    {
        if (*p == '/' || *p == '\0')
        {
            char saved = *p;
            *p = '\0';

            if (mkdir(directory, 0700) != 0 && errno != EEXIST)
                return 0;

            *p = saved;

            if (saved == '\0')
                break;
        }
    }

    char tempPath[560];
    if (snprintf(tempPath, sizeof(tempPath), "%s.tmp", userConfigPath)
        >= (int)sizeof(tempPath))
        return 0;

    FILE *input = fopen(userConfigPath, "r");
    FILE *output = fopen(tempPath, "w");
    if (!output)
    {
        if (input)
            fclose(input);
        return 0;
    }

    int found[8] = {0};
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;

    while (input && (length = getline(&line, &capacity, input)) != -1)
    {
        char *copy = malloc((size_t)length + 1);
        if (!copy)
            goto save_error;

        memcpy(copy, line, (size_t)length + 1);
        char *trimmed = trimWhitespace(copy);
        char *equals = strchr(trimmed, '=');
        int replaced = 0;

        if (equals)
        {
            *equals = '\0';
            char *key = trimWhitespace(trimmed);

            for (int i = 0; i < 8; i++)
            {
                if (strcmp(key, statusbarKeys[i]) == 0)
                {
                    if (fprintf(output, "%s = %s\n",
                                statusbarKeys[i],
                                values[i] ? "true" : "false") < 0)
                    {
                        free(copy);
                        goto save_error;
                    }

                    found[i] = 1;
                    replaced = 1;
                    break;
                }
            }
        }

        free(copy);

        if (!replaced && fwrite(line, 1, (size_t)length, output)
                            != (size_t)length)
            goto save_error;
    }

    if (input && ferror(input))
        goto save_error;

    for (int i = 0; i < 8; i++)
    {
        if (!found[i] &&
            fprintf(output, "%s = %s\n", statusbarKeys[i],
                    values[i] ? "true" : "false") < 0)
            goto save_error;
    }

    free(line);
    if (input)
        fclose(input);

    int writeError = (fflush(output) != 0);
    int closeError = (fclose(output) != 0);

    if (writeError || closeError)
    {
        remove(tempPath);
        return 0;
    }

    if (rename(tempPath, userConfigPath) != 0)
    {
        remove(tempPath);
        return 0;
    }

    editorConfig.statusbarShowVersion = values[0];
    editorConfig.statusbarShowWorkspace = values[1];
    editorConfig.statusbarShowFilename = values[2];
    editorConfig.statusbarShowLanguage = values[3];
    editorConfig.statusbarShowGit = values[4];
    editorConfig.statusbarShowBookmarks = values[5];
    editorConfig.statusbarShowTheme = values[6];
    editorConfig.statusbarShowLineCount = values[7];

    return 1;

save_error:
    free(line);
    if (input)
        fclose(input);
    fclose(output);
    remove(tempPath);
    return 0;
}

int configApplyStatusBarSettings(const int values[], int count)
{
    if (!values || count != 8)
        return 0;

    for (int i = 0; i < count; i++)
    {
        if (values[i] != 0 && values[i] != 1)
            return 0;
    }

    editorConfig.statusbarShowVersion = values[0];
    editorConfig.statusbarShowWorkspace = values[1];
    editorConfig.statusbarShowFilename = values[2];
    editorConfig.statusbarShowLanguage = values[3];
    editorConfig.statusbarShowGit = values[4];
    editorConfig.statusbarShowBookmarks = values[5];
    editorConfig.statusbarShowTheme = values[6];
    editorConfig.statusbarShowLineCount = values[7];

    return 1;
}

int configTabWidth(void) { return editorConfig.tabWidth; }

int configShowLineNumbers(void) { return editorConfig.showLineNumbers; }

const char *configKeymapName(void) { return editorConfig.keymapName; }

int configShowWelcome(void) { return editorConfig.showWelcome; }

int configShowStatusBar(void) { return editorConfig.showStatusBar; }

int configStatusbarShowVersion(void) { return editorConfig.statusbarShowVersion; }

int configStatusbarShowWorkspace(void) { return editorConfig.statusbarShowWorkspace; }

int configStatusbarShowFilename(void) { return editorConfig.statusbarShowFilename; }

int configStatusbarShowLanguage(void) { return editorConfig.statusbarShowLanguage; }

int configStatusbarShowGit(void) { return editorConfig.statusbarShowGit; }

int configStatusbarShowBookmarks(void) { return editorConfig.statusbarShowBookmarks; }

int configStatusbarShowTheme(void) { return editorConfig.statusbarShowTheme; }

int configStatusbarShowLineCount(void) { return editorConfig.statusbarShowLineCount; }
