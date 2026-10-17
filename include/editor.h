#ifndef EDITOR_H
#define EDITOR_H

#define CLAW_VERSION "1.5"

/* Path of the file currently being edited, or NULL for a new buffer. */
extern char *currentFile;

/* Interactive "Save as" prompt state for unnamed buffers. */
typedef struct
{
    int active;
    char filename[256];
    int length;
} SavePrompt;

extern SavePrompt savePrompt;

/* Temporary Git viewer state (Blame, Diff, etc.) */
typedef struct
{
    int active;

    int pluginLog;

    char originalFile[512];

} GitViewState;

extern GitViewState gitView;

/* Manage the editor lifecycle and interactive command loop. */
void editorInit(void);
void editorRun(void);
void editorShutdown(void);

const char *editorStatusMessage(void);
void editorSetStatusMessage(const char *message);

/* Save prompt helpers */
void openSavePrompt(void);
void closeSavePrompt(void);

/* Go to line prompt state and helpers */
typedef struct
{
    int active;
    char input[32];
    int length;
} GotoLinePrompt;

extern GotoLinePrompt gotoPrompt;

void openGotoPrompt(void);
void closeGotoPrompt(void);
void performGotoLine(void);

/* Temporary Git view helpers */
void openGitView(const char *tempFile);
void closeGitView(void);

/* Status Bar customization menu state*/
#define STATUSBAR_OPTION_COUNT 8

typedef struct
{
    int active;
    int selected;
    int original[STATUSBAR_OPTION_COUNT];
    int values[STATUSBAR_OPTION_COUNT];
} StatusBarSettingsMenu;

extern StatusBarSettingsMenu statusBarSettingsMenu;

void openStatusBarSettings(void);
void closeStatusBarSettings(void);
void drawStatusBarSettings(void);
int saveStatusBarSettings(void);

/* Layout Bar customization menu state*/
#define LAYOUT_OPTION_COUNT 5

typedef struct
{
    int active;
    int selected;
    int original[LAYOUT_OPTION_COUNT];
    int values[LAYOUT_OPTION_COUNT];
} LayoutSettingsMenu;

extern LayoutSettingsMenu layoutSettingsMenu;

void openLayoutSettings(void);
void closeLayoutSettings(void);
void drawLayoutSettings(void);
void applyLayoutSettings(void);

/* Custom Theme Maker */
#define THEME_MAKER_COLOR_COUNT 9
#define THEME_MAKER_NAME_SIZE 64

typedef struct
{
    int active;
    int selected;
    char name[THEME_MAKER_NAME_SIZE];
    int nameLength;
    int codes[THEME_MAKER_COLOR_COUNT];
} ThemeMakerMenu;

extern ThemeMakerMenu themeMakerMenu;

void openThemeMaker(void);
void closeThemeMaker(void);
void drawThemeMaker(void);
int saveThemeMaker(void);
void resetThemeMaker(void);

/* Custom Theme Manager */
typedef struct
{
    int active;
    int selected;
    int confirmDelete;
    int pathPromptActive;
    int exportMode;
    int pathLength;
    char path[512];
    char message[160];
} ThemeManagerMenu;

extern ThemeManagerMenu themeManagerMenu;

void openThemeManager(void);
void closeThemeManager(void);
void drawThemeManager(void);

#endif
