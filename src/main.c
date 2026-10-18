#include <string.h>
#include <stdlib.h>
#include "editor.h"
#include "fileio.h"
#include "highlight.h"
#include "recent.h"

int main(int argc, char *argv[])
{
    /* Show the sharp Windows PNG splash before entering terminal raw mode. */
    if (argc == 1 && getenv("WSL_DISTRO_NAME") != NULL)
    {
        int splashResult = system("./tools/launch_splash.sh");

        if (splashResult == 0)
        {
            setenv("CLAW_GRAPHICAL_SPLASH_SHOWN", "1", 1);
        }
    }

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