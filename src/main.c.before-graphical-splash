#include <string.h>
#include "editor.h"
#include "fileio.h"
#include "highlight.h"
#include "recent.h"

int main(int argc, char *argv[])
{
    /* Initialize the terminal and begin with an empty editable buffer. */
    editorInit();

    if (argc > 1)
    {
        /* Loading is optional; without an argument Claw starts a new file. */
        currentFile = strdup(argv[1]);
        if (!openFile(argv[1]))
        {
            editorSetStatusMessage("Could not open the requested file");
        }
        else
        {
            recentAddFile(argv[1]);
            highlightInit();
        }
    }

    /* The editor owns terminal input until the command loop returns. */
    editorRun();
    editorShutdown();

    return 0;
}