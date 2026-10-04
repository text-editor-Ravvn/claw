#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    /* A single editable line stored as bytes plus a terminating NUL. */
    int size;
    char *chars;
    unsigned char *hl;    /* Per-character highlight type (HighlightType). */
    int hlSize;           /* Allocated size of the hl array. */
} Row;

typedef struct
{
    /* The complete document and its persistence state. */
    int numRows;
    Row *rows;
    int endsWithNewline;
    int modified;
} Buffer;

extern Buffer buffer;

/* Create and release the minimum one-row editor buffer. */
void bufferInit(void);
void bufferFree(void);

/* Mutate the document at the current cursor position. */
void insertChar(char c);
void deleteChar(void);
void deleteForward(void);
void insertNewLine(void);

/* Advanced editing operations */
int isOpeningDelimiter(char c);
int isClosingDelimiter(char c);
char matchingDelimiter(char c);
void insertPair(char opening, char closing);
int handleSmartCharacter(char c);

/* Productivity operations */
void duplicateLine(void);
void moveLineUp(void);
void moveLineDown(void);
void toggleCommentLine(void);
void joinLineWithNext(void);

#endif