#ifndef KEYMAP_H
#define KEYMAP_H
#define MOD_CTRL   1
#define MOD_ALT    2
#define MOD_SHIFT  4

/* Editor actions that can be bound to keys. */
enum EditorAction
{
    CMD_NONE = 0,

    CMD_SAVE,

    CMD_GIT_ADD,

    CMD_GIT_RESTORE,

    CMD_GIT_UNSTAGE,

    CMD_GIT_BLAME,

    CMD_PLUGIN_LIST,

    CMD_PLUGIN_RELOAD,

    CMD_PLUGIN_INFO,

    CMD_PLUGIN_HELLO,

    CMD_PLUGIN_STATS,

    CMD_PLUGIN_FORMAT,

    CMD_PLUGIN_STATS_VIEW,

    CMD_PLUGIN_HELP,

    CMD_PLUGIN_CLEAR_LOGS,

    CMD_PLUGIN_LOG,

    CMD_PLUGIN_MANAGER,

    CMD_PLUGIN_TOGGLE,

    CMD_PLUGIN_HEALTH,

    CMD_QUIT,

    CMD_SEARCH,

    CMD_REPLACE,

    CMD_REPLACE_ALL,

    CMD_UNDO,

    CMD_REDO,

    CMD_NEXT_MATCH,

    CMD_PREV_MATCH,

    CMD_DUPLICATE_LINE,

    CMD_MOVE_LINE_UP,

    CMD_MOVE_LINE_DOWN,

    CMD_COMMENT_LINE,

    CMD_JOIN_LINES,

    CMD_UNINDENT,

    CMD_GOTO_BRACKET,

    CMD_WORD_LEFT,

    CMD_WORD_RIGHT,

    CMD_HOME,

    CMD_END,

    CMD_GOTO_LINE,

    CMD_BOOKMARK_TOGGLE,

    CMD_BOOKMARK_NEXT,

    CMD_BOOKMARK_PREV,

    CMD_BOOKMARK_CLEAR,

    CMD_QUICK_SWITCH,

    CMD_FILE_EXPLORER,

    CMD_WORKSPACE_OPEN,

    CMD_SESSION_SAVE,

    CMD_SESSION_RESTORE,

    CMD_THEME_NEXT,

    CMD_STATUSBAR_SETTINGS,

    CMD_LAYOUT_SETTINGS,

    CMD_THEME_MAKER,

    CMD_THEME_MANAGER,

    CMD_PROJECT_BUILD,

    CMD_PROJECT_TEST,

    CMD_PROJECT_OUTPUT,

    CMD_PROJECT_DIAGNOSTICS,
    CMD_PROJECT_DIAGNOSTICS_NEXT,
    CMD_PROJECT_DIAGNOSTICS_PREVIOUS,

    CMD_PROJECT_DASHBOARD,

    /* Phase 8 — IDE Foundation Actions */
    CMD_BUFFER_CLOSE,
    CMD_BUFFER_NEXT,
    CMD_BUFFER_PREV,
    CMD_SPLIT_VERTICAL,
    CMD_SPLIT_HORIZONTAL,
    CMD_PANE_SWITCH,
    CMD_TERMINAL_TOGGLE,
    CMD_SYMBOL_SEARCH,
    CMD_BUFFER_1,
    CMD_BUFFER_2,
    CMD_BUFFER_3,
    CMD_BUFFER_4,
    CMD_BUFFER_5,
    CMD_BUFFER_6,
    CMD_BUFFER_7,
    CMD_BUFFER_8,
    CMD_BUFFER_9,

    /* Phase 9 — Source Control Expansion Actions */
    CMD_GIT_DASHBOARD,
    CMD_GIT_DIFF,
    CMD_GIT_LOG,
    CMD_GIT_COMMIT,
    CMD_GIT_BRANCH,
    CMD_GIT_STASH,

    /* Phase 11 — Intelligent IDE Services Actions */
    CMD_LSP_COMPLETION,
    CMD_LSP_DEFINITION,
    CMD_LSP_HOVER,
    CMD_LSP_REFERENCES,
    CMD_LSP_CODE_ACTION,
    CMD_LSP_DIAGNOSTICS,

    /* Phase 12 — Advanced Platform Services Actions */
    CMD_DAP_START_CONTINUE,
    CMD_DAP_STOP,
    CMD_DAP_PAUSE,
    CMD_DAP_BREAKPOINT_TOGGLE,
    CMD_DAP_STEP_OVER,
    CMD_DAP_STEP_INTO,
    CMD_DAP_STEP_OUT,
    CMD_DAP_DASHBOARD,
    CMD_REMOTE_OPEN,
    CMD_PROJECT_NEW,

    /* Phase 13 — Claw Ecosystem Actions */
    CMD_MARKETPLACE,
    CMD_PACKAGE_MANAGER,
};

#define KEYMAP_MAX_BINDINGS 256

typedef struct
{
    int action;
    int key;
    int modifiers;
} KeyBinding;

/* Populate the binding table with the default key assignments. */
void keymapInit(void);

/* Load key bindings from a keymap config file.
   Returns 1 on success, 0 if the file could not be opened.
   Bindings in the file override existing bindings for the same action. */
int keymapLoad(const char *path);

/* Look up the action bound to a key code.
   Returns the EditorAction, or CMD_NONE if no binding matches. */
int keymapLookup(
    int key,
    int modifiers
);

/* Parse a human-readable key string like "Ctrl+S" into a key code.
   Returns -1 if the string cannot be parsed. */
int keymapParseKey(const char *str);

#endif
