#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "highlight.h"
#include "editor.h"
#include "buffer.h"

extern Buffer buffer;
extern char *currentFile;

/* ── C language definition ──────────────────────────────────────── */

static const char *C_extensions[] = {".c", ".h", ".cc", ".cpp", ".hpp", NULL};

static const char *C_keywords[] = {
    "if", "else", "while", "for", "do", "switch", "case", "default",
    "break", "continue", "return", "goto", "struct", "union", "enum",
    "typedef", "sizeof", "static", "extern", "const", "volatile",
    "register", "inline", "restrict", "_Bool", "_Complex", "_Imaginary",
    NULL};

static const char *C_types[] = {
    "int", "char", "float", "double", "void", "long", "short",
    "unsigned", "signed", "size_t", "ssize_t", "FILE", "NULL",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "bool", "true", "false",
    NULL};

static SyntaxDef C_syntax = {
    .name = "C",
    .extensions = C_extensions,
    .keywords = C_keywords,
    .types = C_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = '#',
    .highlightStrings = 1,
    .highlightNumbers = 1,
};

/* ── Language registry ──────────────────────────────────────────── */

static SyntaxDef *LANGUAGES[] = {
    &C_syntax,
    NULL};

static SyntaxDef *activeSyntax = NULL;

/* ── Helpers ────────────────────────────────────────────────────── */

static int isSeparator(int c)
{
    return isspace(c) || c == '\0' ||
           strchr(",.()+-/*=~%<>[];:{}&|^!?#\"'", c) != NULL;
}

/* ── Public API ─────────────────────────────────────────────────── */

void highlightInit(void)
{
    activeSyntax = NULL;

    if (!currentFile)
        return;

    /* Find the last dot in the filename. */
    const char *dot = strrchr(currentFile, '.');

    if (!dot)
        return;

    for (int i = 0; LANGUAGES[i] != NULL; i++)
    {
        const char **ext = LANGUAGES[i]->extensions;

        for (int j = 0; ext[j] != NULL; j++)
        {
            if (strcmp(dot, ext[j]) == 0)
            {
                activeSyntax = LANGUAGES[i];
                return;
            }
        }
    }
}

const char *highlightLanguageName(void)
{
    if (activeSyntax)
        return activeSyntax->name;

    return "Plain Text";
}

const char *highlightColor(int hlType)
{
    switch (hlType)
    {
    case HL_KEYWORD:
        return "\033[38;5;176m"; /* soft magenta / purple */
    case HL_TYPE:
        return "\033[38;5;80m"; /* teal / cyan */
    case HL_STRING:
        return "\033[38;5;114m"; /* soft green */
    case HL_CHAR_LITERAL:
        return "\033[38;5;114m"; /* same green as strings */
    case HL_NUMBER:
        return "\033[38;5;209m"; /* warm orange */
    case HL_COMMENT:
        return "\033[38;5;102m"; /* dim gray */
    case HL_MULTI_COMMENT:
        return "\033[38;5;102m"; /* dim gray */
    case HL_PREPROCESSOR:
        return "\033[38;5;179m"; /* golden yellow */
    case HL_OPERATOR:
        return "\033[38;5;204m"; /* soft red */
    case HL_MATCH:
        return "\033[30;43m"; /* black on yellow (search) */
    default:
        return "\033[m"; /* reset / normal */
    }
}

void highlightRow(Row *row, int rowIndex)
{
    /* Allocate / resize the hl array to match the row size. */
    if (row->hlSize < row->size)
    {
        unsigned char *newHl = realloc(row->hl, row->size + 1);

        if (!newHl)
            return;

        row->hl = newHl;
        row->hlSize = row->size + 1;
    }

    memset(row->hl, HL_NORMAL, row->size);

    if (!activeSyntax)
        return;

    const char *scs = activeSyntax->singleLineComment;
    const char *mcs = activeSyntax->multiLineCommentStart;
    const char *mce = activeSyntax->multiLineCommentEnd;
    int scsLen = scs ? (int)strlen(scs) : 0;
    int mcsLen = mcs ? (int)strlen(mcs) : 0;
    int mceLen = mce ? (int)strlen(mce) : 0;

    /* Determine whether we start inside a multi-line comment
       by checking the previous row's ending state. */
    int inComment = 0;

    if (rowIndex > 0 && rowIndex <= buffer.numRows)
    {
        /* Scan backwards: check if the previous row ends in a comment.
           We look at the last character's hl type from the previous row. */
        Row *prev = &buffer.rows[rowIndex - 1];

        if (prev->hl && prev->size > 0)
        {
            if (prev->hl[prev->size - 1] == HL_MULTI_COMMENT)
            {
                /* Check if the comment was closed at the very end. */
                if (prev->size >= mceLen &&
                    memcmp(&prev->chars[prev->size - mceLen], mce, mceLen) == 0)
                {
                    inComment = 0;
                }
                else
                {
                    inComment = 1;
                }
            }
        }
    }

    char *chars = row->chars;
    int size = row->size;
    int prevSep = 1; /* Treat start-of-line as a separator. */
    int inString = 0;
    int i = 0;

    /* Handle preprocessor: if the first non-space char is '#'. */
    if (activeSyntax->preprocessorChar && !inComment)
    {
        int j = 0;

        while (j < size && isspace((unsigned char)chars[j]))
            j++;

        if (j < size && chars[j] == activeSyntax->preprocessorChar)
        {
            memset(row->hl, HL_PREPROCESSOR, size);
            return;
        }
    }

    while (i < size)
    {
        unsigned char prevHl = (i > 0) ? row->hl[i - 1] : HL_NORMAL;
        (void)prevHl;

        /* ── Multi-line comment continuation ── */
        if (inComment)
        {
            row->hl[i] = HL_MULTI_COMMENT;

            if (mceLen > 0 && i + mceLen <= size &&
                memcmp(&chars[i], mce, mceLen) == 0)
            {
                memset(&row->hl[i], HL_MULTI_COMMENT, mceLen);
                i += mceLen;
                inComment = 0;
                prevSep = 1;
                continue;
            }

            i++;
            prevSep = 0;
            continue;
        }

        /* ── String continuation ── */
        if (inString)
        {
            row->hl[i] = HL_STRING;

            if (chars[i] == '\\' && i + 1 < size)
            {
                row->hl[i + 1] = HL_STRING;
                i += 2;
                continue;
            }

            if (chars[i] == inString)
                inString = 0;

            i++;
            prevSep = 1;
            continue;
        }

        /* ── Single-line comment ── */
        if (scsLen > 0 && i + scsLen <= size &&
            memcmp(&chars[i], scs, scsLen) == 0)
        {
            memset(&row->hl[i], HL_COMMENT, size - i);
            return; /* Rest of line is comment. */
        }

        /* ── Multi-line comment start ── */
        if (mcsLen > 0 && i + mcsLen <= size &&
            memcmp(&chars[i], mcs, mcsLen) == 0)
        {
            memset(&row->hl[i], HL_MULTI_COMMENT, mcsLen);
            i += mcsLen;
            inComment = 1;
            continue;
        }

        /* ── String / char literal start ── */
        if (activeSyntax->highlightStrings &&
            (chars[i] == '"' || chars[i] == '\''))
        {
            int quoteType = chars[i];

            if (quoteType == '\'')
                row->hl[i] = HL_CHAR_LITERAL;
            else
                row->hl[i] = HL_STRING;

            inString = quoteType;
            i++;
            prevSep = 0;
            continue;
        }

        /* ── Numbers ── */
        if (activeSyntax->highlightNumbers)
        {
            if ((isdigit((unsigned char)chars[i]) && prevSep) ||
                (chars[i] == '.' && i + 1 < size &&
                 isdigit((unsigned char)chars[i + 1]) && prevSep))
            {
                while (i < size &&
                       (isdigit((unsigned char)chars[i]) ||
                        chars[i] == '.' ||
                        chars[i] == 'x' || chars[i] == 'X' ||
                        (chars[i] >= 'a' && chars[i] <= 'f') ||
                        (chars[i] >= 'A' && chars[i] <= 'F') ||
                        chars[i] == 'L' || chars[i] == 'l' ||
                        chars[i] == 'U' || chars[i] == 'u'))
                {
                    row->hl[i] = HL_NUMBER;
                    i++;
                }

                prevSep = 0;
                continue;
            }
        }

        /* ── Keywords and types ── */
        if (prevSep)
        {
            int matched = 0;

            /* Check type keywords first. */
            if (activeSyntax->types)
            {
                for (int k = 0; activeSyntax->types[k]; k++)
                {
                    int kwLen = (int)strlen(activeSyntax->types[k]);

                    if (i + kwLen <= size &&
                        memcmp(&chars[i], activeSyntax->types[k], kwLen) == 0 &&
                        (i + kwLen == size ||
                         isSeparator((unsigned char)chars[i + kwLen])))
                    {
                        memset(&row->hl[i], HL_TYPE, kwLen);
                        i += kwLen;
                        prevSep = 0;
                        matched = 1;
                        break;
                    }
                }
            }

            if (!matched && activeSyntax->keywords)
            {
                for (int k = 0; activeSyntax->keywords[k]; k++)
                {
                    int kwLen = (int)strlen(activeSyntax->keywords[k]);

                    if (i + kwLen <= size &&
                        memcmp(&chars[i], activeSyntax->keywords[k], kwLen) == 0 &&
                        (i + kwLen == size ||
                         isSeparator((unsigned char)chars[i + kwLen])))
                    {
                        memset(&row->hl[i], HL_KEYWORD, kwLen);
                        i += kwLen;
                        prevSep = 0;
                        matched = 1;
                        break;
                    }
                }
            }

            if (matched)
                continue;
        }

        /* ── Operators ── */
        if (strchr("+-*/%=<>!&|^~?:", chars[i]))
        {
            row->hl[i] = HL_OPERATOR;
            i++;
            prevSep = 1;
            continue;
        }

        prevSep = isSeparator((unsigned char)chars[i]);
        i++;
    }
}
