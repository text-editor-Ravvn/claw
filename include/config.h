#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    /* Number of spaces inserted when the Tab key is pressed. */
    int tabWidth;

    /* Whether line numbers are displayed in the gutter. */
    int showLineNumbers;

    /* Whether the welcome screen is shown for empty buffers. */
    int showWelcome;

    /* Whether the status bar is drawn at the bottom. */
    int showStatusBar;

    /* Name of the active keymap file (without path or extension). */
    char keymapName[64];

} EditorConfig;

extern EditorConfig editorConfig;

/* Set all settings to their compiled-in defaults. */
void configInit(void);

/* Load settings from a key=value config file.
   Returns 1 on success, 0 if the file could not be opened.
   Unknown keys are silently ignored. */
int configLoad(const char *path);

/* Typed accessors for the current configuration. */
int configTabWidth(void);
int configShowLineNumbers(void);
int configShowWelcome(void);
int configShowStatusBar(void);
const char *configKeymapName(void);

#endif
