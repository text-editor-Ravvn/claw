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
    HL_MATCH,
    HL_BRACKET_MATCH
} HighlightType;

/* Multi-line open states for cross-row continuation */
enum
{
    HL_STATE_NONE = 0,
    HL_STATE_BLOCK_COMMENT,
    HL_STATE_TRIPLE_DOUBLE,
    HL_STATE_TRIPLE_SINGLE,
    HL_STATE_BACKTICK,
    HL_STATE_MD_CODEBLOCK
};

/* Language capability flags */
enum
{
    LANG_FLAG_NONE          = 0,
    LANG_FLAG_DOCSTRINGS    = 1 << 0, /* Python """ and ''' multi-line docstrings */
    LANG_FLAG_TEMPLATES     = 1 << 1, /* JS/TS/Go backtick `...` template/raw strings */
    LANG_FLAG_RUST_MACROS   = 1 << 2, /* Rust macros ending with ! (e.g. println!, vec!) */
    LANG_FLAG_RUST_LIFETIME = 1 << 3, /* Rust lifetimes 'a */
    LANG_FLAG_MARKDOWN      = 1 << 4, /* Markdown headers, lists, links, code blocks */
    LANG_FLAG_YAML          = 1 << 5, /* YAML key-value pairs */
    LANG_FLAG_JS_REGEX      = 1 << 6  /* JavaScript / TypeScript regex literals /pattern/flags */
};

/* Language definition for syntax-aware highlighting. */
typedef struct SyntaxDef
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
    const char *commentPrefix;
    const char *commentSuffix;
    char indentTrigger;
    int flags;
} SyntaxDef;

/* Detect the language from the current filename extension or shebang. */
void highlightInit(void);

/* Compute the per-character hl array for a single row.
   rowIndex is used for multi-line comment and string state tracking. */
void highlightRow(Row *row, int rowIndex);

/* Return the ANSI color escape string for a given highlight type. */
const char *highlightColor(int hlType);

/* Return the human-readable name of the detected language, or "Plain Text". */
const char *highlightLanguageName(void);

/* Phase 10: Language Registry & Dynamic Detection APIs */
int highlightRegisterLanguage(SyntaxDef *syntax);
int highlightLanguageCount(void);
const SyntaxDef *highlightGetLanguage(int index);
const SyntaxDef *highlightFindLanguageByName(const char *name);
const SyntaxDef *highlightFindLanguageByFilename(const char *filename);
const SyntaxDef *highlightDetectLanguageFromContent(const char *firstLine);
int highlightSetLanguage(const char *name);
const SyntaxDef *highlightGetActiveSyntax(void);
const char *highlightGetCommentPrefix(void);
const char *highlightGetCommentSuffix(void);
char highlightGetIndentTrigger(void);

#endif

