#include "editor.h"
#include "fileio.h"

int main(int argc, char *argv[])
{
    editorInit();

    if (argc > 1)
    {
        currentFile = argv[1];
        openFile(argv[1]);
    }

    editorRun();

    return 0;
}