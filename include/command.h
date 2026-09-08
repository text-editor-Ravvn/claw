#ifndef COMMAND_H
#define COMMAND_H

#include "keymap.h"

/* Execute the editor action identified by the given EditorAction enum value.
   This is the single dispatch point between key bindings and editor functions. */
void commandExecute(int action);

#endif
