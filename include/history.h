#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX 1000

typedef struct
{
    int type;

    int row;
    int col;

    char ch;

} EditAction;

void historyInit(void);

#endif