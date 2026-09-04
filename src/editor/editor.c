#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "input.h"
#include "render.h"
#include "cursor.h"
#include "rawmode.h"
#include "buffer.h"
#include "fileio.h"
char *currentFile = NULL;

void editorInit(void)
{
    enableRawMode();

    bufferInit();
}

void editorRun(void)
{
    while(1)
    {
        refreshScreen();

        int key = readKey();

      switch(key)
{
    case ARROW_UP:
        moveCursorUp();
        break;

    case ARROW_DOWN:
        moveCursorDown();
        break;

    case ARROW_LEFT:
        moveCursorLeft();
        break;

    case ARROW_RIGHT:
        moveCursorRight();
        break;

    case '\r':
    case '\n':
        insertNewLine();
        break;
    
    case CTRL_KEY('s'):
    printf("\nSAVE PRESSED\n");

    if(currentFile)
        saveFile(currentFile);

    break;

    case 127:
        deleteChar();
        break;

    case 'q':
        return;

    default:

        if (key >= 32 && key <= 126)
            insertChar(key);

        break;
}
    }
}