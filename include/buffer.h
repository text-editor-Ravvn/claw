#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    int size;
    char *chars;
} Row;

typedef struct
{
    int numRows;
    Row *rows;
    int endsWithNewline;
} Buffer;

extern Buffer buffer;

void bufferInit(void);
void bufferFree(void);

void insertChar(char c);
void deleteChar(void);
void deleteForward(void);
void insertNewLine(void);
#endif