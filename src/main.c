#include <string.h>
#include <stdlib.h>
#include "editor.h"
#include "fileio.h"
#include "highlight.h"
#include "recent.h"
#include "buffer_manager.h"

int main(int argc, char *argv[])
{


    /* Initialize the terminal and begin with an empty editable buffer. */
    editorInit();

    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (bufferManagerOpen(argv[i]) < 0)
            {
                editorSetStatusMessage("Could not open the requested file");
            }
            else
            {
                recentAddFile(argv[i]);
            }
        }
        bufferManagerSwitchTo(0);
        highlightInit();
    }

    /* The editor owns terminal input until the command loop returns. */
    editorRun();
    editorShutdown();

    return 0;
}