#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

#include "buffer.h"

/* Token categories produced by the syntax scanner. */
typedef enum
{
    HL_NORMAL = 0,
    HL_KEYWORD,
    HL_TYPE,
    HL_STRING,
    HL_CHAR_LITERAL,
    HL_NUMBER,
    HL_COMMENT,
    HL_MULTI_COMMENT,
    HL_PREPROCESSOR,
    HL_OPERATOR,
    HL_MATCH
} HighlightType;

/* Language definition for syntax-aware highlighting. */
typedef struct
{
    const char *name;
    const char **extensions;
    const char **keywords;
    const char **types;
    const char *singleLineComment;
    const char *multiLineCommentStart;
    const char *multiLineCommentEnd;
    char preprocessorChar;
    int highlightStrings;
    int highlightNumbers;
} SyntaxDef;

/* Detect the language from the current filename extension. */
void highlightInit(void);

/* Compute the per-character hl array for a single row.
   rowIndex is used for multi-line comment state tracking. */
void highlightRow(Row *row, int rowIndex);

/* Return the ANSI color escape string for a given highlight type. */
const char *highlightColor(int hlType);

/* Return the human-readable name of the detected language, or "Plain Text". */
const char *highlightLanguageName(void);

#endif
