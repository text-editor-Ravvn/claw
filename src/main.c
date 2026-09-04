#include "editor.h"
#include "fileio.h"

int main(int argc, char *argv[])
{
    /* Initialize the terminal and begin with an empty editable buffer. */
    editorInit();

    if (argc > 1)
    {
        /* Loading is optional; without an argument Claw starts a new file. */
        currentFile = argv[1];
        openFile(argv[1]);
    }

    /* The editor owns terminal input until the command loop returns. */
    editorRun();
    editorShutdown();

    return 0;
}