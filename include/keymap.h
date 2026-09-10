#ifndef KEYMAP_H
#define KEYMAP_H

/* Editor actions that can be bound to keys. */
enum EditorAction
{
    CMD_NONE = 0,

    CMD_SAVE,

    CMD_GIT_ADD,

    CMD_GIT_RESTORE,

    CMD_GIT_UNSTAGE,

    CMD_GIT_BLAME,

    CMD_QUIT,

    CMD_SEARCH,

    CMD_REPLACE,

    CMD_REPLACE_ALL,

    CMD_UNDO,

    CMD_REDO,

    CMD_NEXT_MATCH,

    CMD_PREV_MATCH
};

#define KEYMAP_MAX_BINDINGS 32

typedef struct
{
    int action;
    int key;
} KeyBinding;

/* Populate the binding table with the default key assignments. */
void keymapInit(void);

/* Load key bindings from a keymap config file.
   Returns 1 on success, 0 if the file could not be opened.
   Bindings in the file override existing bindings for the same action. */
int keymapLoad(const char *path);

/* Look up the action bound to a key code.
   Returns the EditorAction, or CMD_NONE if no binding matches. */
int keymapLookup(int key);

/* Parse a human-readable key string like "Ctrl+S" into a key code.
   Returns -1 if the string cannot be parsed. */
int keymapParseKey(const char *str);

#endif
