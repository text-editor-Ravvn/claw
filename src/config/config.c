#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "utils.h"

EditorConfig editorConfig;

void configInit(void) {
  /* Compiled-in defaults matching a reasonable editing experience. */
  editorConfig.tabWidth = 4;
  editorConfig.showLineNumbers = 0;
  editorConfig.showWelcome = 1;
  editorConfig.showStatusBar = 1;
  snprintf(editorConfig.keymapName, sizeof(editorConfig.keymapName), "default");
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

int configTabWidth(void) { return editorConfig.tabWidth; }

int configShowLineNumbers(void) { return editorConfig.showLineNumbers; }

const char *configKeymapName(void) { return editorConfig.keymapName; }

int configShowWelcome(void) { return editorConfig.showWelcome; }

int configShowStatusBar(void) { return editorConfig.showStatusBar; }
