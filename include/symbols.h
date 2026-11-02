#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "buffer.h"

#define SYMBOLS_MAX 512
#define SYMBOL_NAME_MAX 128
#define SYMBOL_PATH_MAX 512
#define SYMBOL_SIG_MAX 160

typedef enum
{
    SYMBOL_FUNCTION = 0,
    SYMBOL_STRUCT,
    SYMBOL_ENUM,
    SYMBOL_MACRO,
    SYMBOL_TYPEDEF,
    SYMBOL_VARIABLE,
    SYMBOL_OTHER
} SymbolKind;

typedef struct
{
    char name[SYMBOL_NAME_MAX];
    char file[SYMBOL_PATH_MAX];
    int line; /* 1-indexed */
    SymbolKind kind;
    char signature[SYMBOL_SIG_MAX];
} Symbol;

typedef struct
{
    int active;
    Symbol symbols[SYMBOLS_MAX];
    int count;
    char query[128];
    int queryLen;
    int filteredIndices[SYMBOLS_MAX];
    int filteredCount;
    int selectedIndex;
} SymbolIndexState;

extern SymbolIndexState symbolState;

/* Initialize symbols subsystem */
void symbolsInit(void);

/* Open symbols modal search view */
void symbolsOpen(void);

/* Close symbols modal view */
void symbolsClose(void);

/* Check if symbols modal view is active */
int symbolsIsActive(void);

/* Clear indexed symbols */
void symbolsClear(void);

/* Add a symbol definition */
void symbolsAdd(const char *name, const char *file, int line, SymbolKind kind, const char *sig);

/* Index symbols from a single buffer document */
void symbolsIndexBuffer(const Buffer *buf, const char *filename);

/* Index symbols from open buffers and workspace files */
void symbolsIndexWorkspace(void);

/* Filter symbols based on current search query */
void symbolsFilter(void);

/* Select next match */
void symbolsSelectNext(void);

/* Select previous match */
void symbolsSelectPrev(void);

/* Add character to query */
void symbolsAddChar(char c);

/* Remove last character from query */
void symbolsBackspace(void);

/* Get currently selected symbol, or NULL if none */
const Symbol *symbolsGetSelected(void);

/* Jump editor to selected symbol location. Returns 1 on success, 0 on failure. */
int symbolsJumpToSelected(void);

/* Convert symbol kind to readable badge string */
const char *symbolKindToString(SymbolKind kind);

#endif
