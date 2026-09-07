#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX 1000

#define ACTION_INSERT 1
#define ACTION_DELETE 2
#define ACTION_NEWLINE 3

typedef struct
{
    int type;

    int row;
    int col;

    char ch;

} EditAction;

void historyInit(void);

void historyPushInsert(
    int row,
    int col,
    char ch
);

void historyPushDelete(
    int row,
    int col,
    char ch
);

void historyPushNewLine(
    int row,
    int col
);

int historyUndo(void);

void historyRedo(void);

void historySetRecording(int enabled);

#endif