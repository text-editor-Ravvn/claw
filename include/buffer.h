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
} Buffer;

extern Buffer buffer;

void bufferInit(void);

void insertChar(char c);
void deleteChar(void);
void insertNewLine(void);
#endif