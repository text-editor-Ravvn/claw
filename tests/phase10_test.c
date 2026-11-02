#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "highlight.h"
#include "buffer.h"
#include "buffer_manager.h"
#include "cursor.h"
#include "config.h"

#include "viewport.h"
#include "split.h"

extern Buffer buffer;
extern Cursor cursor;
char *currentFile = NULL;

Viewport viewport = {24, 80, 0, 0, 0};
SplitLayout splitLayout = {0};

/* Dummy search, git & split functions required by row.c / buffer_manager.c */
void refreshSearchMatches(void) {}
void gitRefreshStatus(const char *f) { (void)f; }
int openFile(const char *filename) { (void)filename; return 1; }
int saveFile(const char *filename) { (void)filename; return 1; }
int splitIsActive(void) { return 0; }

static void resetTestBuffer(void)
{
    bufferFree();
    bufferInit();
    cursor.x = 0;
    cursor.y = 0;
    if (currentFile)
    {
        free(currentFile);
        currentFile = NULL;
    }
}

static void setRowContent(int rowIndex, const char *text)
{
    while (buffer.numRows <= rowIndex)
    {
        Row *newRows = realloc(buffer.rows, sizeof(Row) * (buffer.numRows + 1));
        assert(newRows != NULL);
        buffer.rows = newRows;
        buffer.rows[buffer.numRows].size = 0;
        buffer.rows[buffer.numRows].chars = malloc(1);
        buffer.rows[buffer.numRows].chars[0] = '\0';
        buffer.rows[buffer.numRows].hl = NULL;
        buffer.rows[buffer.numRows].hlSize = 0;
        buffer.rows[buffer.numRows].hlOpenState = 0;
        buffer.numRows++;
    }

    Row *row = &buffer.rows[rowIndex];
    free(row->chars);
    free(row->hl);
    row->hl = NULL;
    row->hlSize = 0;
    row->hlOpenState = 0;

    int len = (int)strlen(text);
    row->size = len;
    row->chars = malloc((size_t)len + 1);
    memcpy(row->chars, text, len + 1);
}

/* ── Test Suite 1: Language Registry & Detection ────────────────── */

static void test_language_registry_and_detection(void)
{
    printf("[1/10] Testing Language Registry & Dynamic Detection...\n");

    highlightInit();
    int count = highlightLanguageCount();
    assert(count >= 13);

    /* Verify essential languages exist */
    assert(highlightFindLanguageByName("C") != NULL);
    assert(highlightFindLanguageByName("C++") != NULL);
    assert(highlightFindLanguageByName("Python") != NULL);
    assert(highlightFindLanguageByName("JavaScript") != NULL);
    assert(highlightFindLanguageByName("TypeScript") != NULL);
    assert(highlightFindLanguageByName("Rust") != NULL);
    assert(highlightFindLanguageByName("Go") != NULL);
    assert(highlightFindLanguageByName("Java") != NULL);
    assert(highlightFindLanguageByName("JSON") != NULL);
    assert(highlightFindLanguageByName("YAML") != NULL);
    assert(highlightFindLanguageByName("Markdown") != NULL);
    assert(highlightFindLanguageByName("Shell") != NULL);
    assert(highlightFindLanguageByName("HTML") != NULL);

    /* Case-insensitive lookup */
    assert(highlightFindLanguageByName("python") != NULL);
    assert(highlightFindLanguageByName("RUST") != NULL);
    assert(highlightFindLanguageByName("javascript") != NULL);

    /* Filename extension lookup */
    assert(strcmp(highlightFindLanguageByFilename("main.c")->name, "C") == 0);
    assert(strcmp(highlightFindLanguageByFilename("server.cpp")->name, "C++") == 0);
    assert(strcmp(highlightFindLanguageByFilename("script.py")->name, "Python") == 0);
    assert(strcmp(highlightFindLanguageByFilename("app.js")->name, "JavaScript") == 0);
    assert(strcmp(highlightFindLanguageByFilename("index.ts")->name, "TypeScript") == 0);
    assert(strcmp(highlightFindLanguageByFilename("lib.rs")->name, "Rust") == 0);
    assert(strcmp(highlightFindLanguageByFilename("handler.go")->name, "Go") == 0);
    assert(strcmp(highlightFindLanguageByFilename("App.java")->name, "Java") == 0);
    assert(strcmp(highlightFindLanguageByFilename("data.json")->name, "JSON") == 0);
    assert(strcmp(highlightFindLanguageByFilename("deploy.yaml")->name, "YAML") == 0);
    assert(strcmp(highlightFindLanguageByFilename("README.md")->name, "Markdown") == 0);
    assert(strcmp(highlightFindLanguageByFilename("build.sh")->name, "Shell") == 0);
    assert(strcmp(highlightFindLanguageByFilename("page.html")->name, "HTML") == 0);

    /* Special files */
    assert(strcmp(highlightFindLanguageByFilename("Makefile")->name, "Shell") == 0);
    assert(strcmp(highlightFindLanguageByFilename("Dockerfile")->name, "Shell") == 0);

    /* Shebang detection */
    assert(strcmp(highlightDetectLanguageFromContent("#!/usr/bin/env python3")->name, "Python") == 0);
    assert(strcmp(highlightDetectLanguageFromContent("#!/bin/bash")->name, "Shell") == 0);
    assert(strcmp(highlightDetectLanguageFromContent("#!/usr/bin/node")->name, "JavaScript") == 0);

    /* Manual language setting */
    assert(highlightSetLanguage("Rust") == 1);
    assert(strcmp(highlightLanguageName(), "Rust") == 0);
    assert(highlightSetLanguage("Python") == 1);
    assert(strcmp(highlightLanguageName(), "Python") == 0);
    assert(highlightSetLanguage("Plain Text") == 1);
    assert(strcmp(highlightLanguageName(), "Plain Text") == 0);

    /* Dynamic custom language registration */
    static const char *customExt[] = {".clawlang", NULL};
    static const char *customKw[] = {"claw", "editor", NULL};
    static SyntaxDef customSyntax = {
        .name = "ClawLang",
        .extensions = customExt,
        .keywords = customKw,
        .types = NULL,
        .singleLineComment = "#",
        .multiLineCommentStart = NULL,
        .multiLineCommentEnd = NULL,
        .preprocessorChar = 0,
        .highlightStrings = 1,
        .highlightNumbers = 1,
        .commentPrefix = "# ",
        .commentSuffix = NULL,
        .indentTrigger = ':',
        .flags = LANG_FLAG_NONE,
    };
    assert(highlightRegisterLanguage(&customSyntax) == 1);
    assert(highlightFindLanguageByName("ClawLang") != NULL);
    assert(strcmp(highlightFindLanguageByFilename("test.clawlang")->name, "ClawLang") == 0);

    printf("  -> Language Registry & Detection PASSED.\n");
}

/* ── Test Suite 2: Python Highlighting & Multiline Docstrings ───── */

static void test_python_highlighting(void)
{
    printf("[2/10] Testing Python Syntax Highlighting & Multiline Docstrings...\n");

    resetTestBuffer();
    currentFile = strdup("test.py");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "Python") == 0);

    /* 1. Keywords, types, numbers, single-line comment */
    setRowContent(0, "def calculate(x: int) -> int: # Function def");
    highlightRow(&buffer.rows[0], 0);

    /* "def" should be HL_KEYWORD (indices 0..2) */
    assert(buffer.rows[0].hl[0] == HL_KEYWORD);
    assert(buffer.rows[0].hl[1] == HL_KEYWORD);
    assert(buffer.rows[0].hl[2] == HL_KEYWORD);
    assert(buffer.rows[0].hl[3] == HL_NORMAL);

    /* "int" should be HL_TYPE (indices 17..19) */
    assert(buffer.rows[0].hl[17] == HL_TYPE);
    assert(buffer.rows[0].hl[18] == HL_TYPE);
    assert(buffer.rows[0].hl[19] == HL_TYPE);

    /* "# Function def" should be HL_COMMENT */
    char *commentPos = strchr(buffer.rows[0].chars, '#');
    int cIdx = (int)(commentPos - buffer.rows[0].chars);
    for (int i = cIdx; i < buffer.rows[0].size; i++)
    {
        assert(buffer.rows[0].hl[i] == HL_COMMENT);
    }

    /* 2. Number formats */
    setRowContent(1, "val = 0x1F + 0b1010 + 42 + 3.14e-2");
    highlightRow(&buffer.rows[1], 1);
    /* 0x1F should be HL_NUMBER */
    int hexIdx = (int)(strstr(buffer.rows[1].chars, "0x1F") - buffer.rows[1].chars);
    assert(buffer.rows[1].hl[hexIdx] == HL_NUMBER);
    assert(buffer.rows[1].hl[hexIdx + 3] == HL_NUMBER);

    /* 3. Multi-line docstrings """ across rows */
    setRowContent(2, "\"\"\"Start of docstring");
    setRowContent(3, "middle text of docstring");
    setRowContent(4, "end of docstring\"\"\"");
    setRowContent(5, "x = 100");

    highlightRow(&buffer.rows[2], 2);
    assert(buffer.rows[2].hl[0] == HL_STRING);
    assert(buffer.rows[2].hlOpenState == HL_STATE_TRIPLE_DOUBLE);

    highlightRow(&buffer.rows[3], 3);
    assert(buffer.rows[3].hl[0] == HL_STRING);
    assert(buffer.rows[3].hlOpenState == HL_STATE_TRIPLE_DOUBLE);

    highlightRow(&buffer.rows[4], 4);
    assert(buffer.rows[4].hl[0] == HL_STRING);
    assert(buffer.rows[4].hlOpenState == HL_STATE_NONE);

    highlightRow(&buffer.rows[5], 5);
    /* x = 100: x is NORMAL, 100 is NUMBER */
    assert(buffer.rows[5].hl[0] == HL_NORMAL);
    int numPos = (int)(strstr(buffer.rows[5].chars, "100") - buffer.rows[5].chars);
    assert(buffer.rows[5].hl[numPos] == HL_NUMBER);
    assert(buffer.rows[5].hlOpenState == HL_STATE_NONE);

    /* 4. Escaped quotes inside docstrings */
    setRowContent(6, "\"\"\"Doc with \\\"\\\"\\\" escaped quote\"\"\"");
    highlightRow(&buffer.rows[6], 6);
    assert(buffer.rows[6].hl[0] == HL_STRING);
    assert(buffer.rows[6].hlOpenState == HL_STATE_NONE);

    printf("  -> Python Highlighting & Docstrings PASSED.\n");
}

/* ── Test Suite 3: JavaScript & TypeScript Highlighting ─────────── */

static void test_js_ts_highlighting(void)
{
    printf("[3/10] Testing JavaScript & TypeScript Syntax & Template Literals...\n");

    /* JavaScript */
    resetTestBuffer();
    currentFile = strdup("app.js");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "JavaScript") == 0);

    setRowContent(0, "const greeting = `Hello,");
    setRowContent(1, "World!`;");
    setRowContent(2, "function add(a, b) { return a + b; }");

    highlightRow(&buffer.rows[0], 0);
    /* `const` is keyword */
    assert(buffer.rows[0].hl[0] == HL_KEYWORD);
    assert(buffer.rows[0].hl[4] == HL_KEYWORD);
    assert(buffer.rows[0].hl[5] == HL_NORMAL);
    /* Template literal backtick open */
    int bIdx = (int)(strchr(buffer.rows[0].chars, '`') - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[bIdx] == HL_STRING);
    assert(buffer.rows[0].hlOpenState == HL_STATE_BACKTICK);

    /* Row 1 continuation */
    highlightRow(&buffer.rows[1], 1);
    assert(buffer.rows[1].hl[0] == HL_STRING);
    int closeIdx = (int)(strchr(buffer.rows[1].chars, '`') - buffer.rows[1].chars);
    assert(buffer.rows[1].hl[closeIdx] == HL_STRING);
    assert(buffer.rows[1].hlOpenState == HL_STATE_NONE);

    /* Row 2: function & return */
    highlightRow(&buffer.rows[2], 2);
    assert(buffer.rows[2].hl[0] == HL_KEYWORD); /* function */
    int retIdx = (int)(strstr(buffer.rows[2].chars, "return") - buffer.rows[2].chars);
    assert(buffer.rows[2].hl[retIdx] == HL_KEYWORD);

    /* JS Regex Literal vs Division Operator */
    resetTestBuffer();
    currentFile = strdup("regex.js");
    highlightInit();

    setRowContent(0, "const pattern = /[a-z0-9]+/gi;");
    setRowContent(1, "let result = 100 / 5;");
    setRowContent(2, "return /test\\/slash/;");

    highlightRow(&buffer.rows[0], 0);
    int reIdx = (int)(strstr(buffer.rows[0].chars, "/[a-z0-9]+/gi") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[reIdx] == HL_STRING);
    assert(buffer.rows[0].hl[reIdx + 1] == HL_STRING);
    assert(buffer.rows[0].hl[reIdx + 12] == HL_STRING); /* 'i' */
    assert(buffer.rows[0].hl[reIdx + 13] == HL_NORMAL); /* ';' */

    highlightRow(&buffer.rows[1], 1);
    int divIdx = (int)(strchr(buffer.rows[1].chars, '/') - buffer.rows[1].chars);
    assert(buffer.rows[1].hl[divIdx] == HL_OPERATOR);

    highlightRow(&buffer.rows[2], 2);
    int retReIdx = (int)(strstr(buffer.rows[2].chars, "/test\\/slash/") - buffer.rows[2].chars);
    assert(buffer.rows[2].hl[retReIdx] == HL_STRING);

    /* TypeScript */
    resetTestBuffer();
    currentFile = strdup("module.ts");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "TypeScript") == 0);

    setRowContent(0, "interface User { id: number; name: string; }");
    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_KEYWORD); /* interface */
    int numTypeIdx = (int)(strstr(buffer.rows[0].chars, "number") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[numTypeIdx] == HL_TYPE);
    int strTypeIdx = (int)(strstr(buffer.rows[0].chars, "string") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[strTypeIdx] == HL_TYPE);

    printf("  -> JavaScript & TypeScript Highlighting PASSED.\n");
}

/* ── Test Suite 4: Rust Highlighting (Macros & Lifetimes) ───────── */

static void test_rust_highlighting(void)
{
    printf("[4/10] Testing Rust Syntax (Keywords, Macros, Lifetimes, Attributes)...\n");

    resetTestBuffer();
    currentFile = strdup("main.rs");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "Rust") == 0);

    /* 1. Attribute #[derive(Debug)] */
    setRowContent(0, "#[derive(Debug)]");
    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_PREPROCESSOR);
    assert(buffer.rows[0].hl[1] == HL_PREPROCESSOR);
    assert(buffer.rows[0].hl[8] == HL_PREPROCESSOR);

    /* 2. Macro println!("Hello") */
    setRowContent(1, "println!(\"Hello\");");
    highlightRow(&buffer.rows[1], 1);
    /* "println!" is HL_KEYWORD */
    assert(buffer.rows[1].hl[0] == HL_KEYWORD);
    assert(buffer.rows[1].hl[6] == HL_KEYWORD);
    assert(buffer.rows[1].hl[7] == HL_KEYWORD); /* ! */
    /* "Hello" is HL_STRING */
    int quotePos = (int)(strchr(buffer.rows[1].chars, '"') - buffer.rows[1].chars);
    assert(buffer.rows[1].hl[quotePos] == HL_STRING);

    /* 3. Lifetime &'a str */
    setRowContent(2, "fn parse<'a>(text: &'a str) -> &'a str {");
    highlightRow(&buffer.rows[2], 2);
    /* 'a should be HL_TYPE */
    char *life = strstr(buffer.rows[2].chars, "'a");
    assert(life != NULL);
    int lifeIdx = (int)(life - buffer.rows[2].chars);
    assert(buffer.rows[2].hl[lifeIdx] == HL_TYPE);
    assert(buffer.rows[2].hl[lifeIdx + 1] == HL_TYPE);

    /* 4. Types i32, Vec, Option */
    setRowContent(3, "let mut v: Vec<i32> = Vec::new();");
    highlightRow(&buffer.rows[3], 3);
    assert(buffer.rows[3].hl[0] == HL_KEYWORD); /* let */
    int vecIdx = (int)(strstr(buffer.rows[3].chars, "Vec") - buffer.rows[3].chars);
    assert(buffer.rows[3].hl[vecIdx] == HL_TYPE);
    int i32Idx = (int)(strstr(buffer.rows[3].chars, "i32") - buffer.rows[3].chars);
    assert(buffer.rows[3].hl[i32Idx] == HL_TYPE);

    printf("  -> Rust Highlighting PASSED.\n");
}

/* ── Test Suite 5: Go & Java Highlighting ───────────────────────── */

static void test_go_and_java_highlighting(void)
{
    printf("[5/10] Testing Go (Raw Strings) & Java Highlighting...\n");

    /* Go */
    resetTestBuffer();
    currentFile = strdup("server.go");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "Go") == 0);

    setRowContent(0, "package main");
    setRowContent(1, "func main() {");
    setRowContent(2, "    raw := `line1");
    setRowContent(3, "line2`");

    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_KEYWORD); /* package */

    highlightRow(&buffer.rows[1], 1);
    assert(buffer.rows[1].hl[0] == HL_KEYWORD); /* func */

    highlightRow(&buffer.rows[2], 2);
    int rawIdx = (int)(strchr(buffer.rows[2].chars, '`') - buffer.rows[2].chars);
    assert(buffer.rows[2].hl[rawIdx] == HL_STRING);
    assert(buffer.rows[2].hlOpenState == HL_STATE_BACKTICK);

    highlightRow(&buffer.rows[3], 3);
    assert(buffer.rows[3].hl[0] == HL_STRING);
    assert(buffer.rows[3].hlOpenState == HL_STATE_NONE);

    /* Java */
    resetTestBuffer();
    currentFile = strdup("Application.java");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "Java") == 0);

    setRowContent(0, "public class Application extends Base {");
    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_KEYWORD); /* public */
    int clsIdx = (int)(strstr(buffer.rows[0].chars, "class") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[clsIdx] == HL_KEYWORD);
    int extIdx = (int)(strstr(buffer.rows[0].chars, "extends") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[extIdx] == HL_KEYWORD);

    printf("  -> Go & Java Highlighting PASSED.\n");
}

/* ── Test Suite 6: Structured Data (JSON, YAML, Markdown) ───────── */

static void test_structured_data_languages(void)
{
    printf("[6/10] Testing Structured Data (JSON, YAML, Markdown)...\n");

    /* JSON */
    resetTestBuffer();
    currentFile = strdup("config.json");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "JSON") == 0);

    setRowContent(0, "{ \"name\": \"claw\", \"active\": true, \"count\": 42 }");
    highlightRow(&buffer.rows[0], 0);
    int trueIdx = (int)(strstr(buffer.rows[0].chars, "true") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[trueIdx] == HL_KEYWORD);
    int countIdx = (int)(strstr(buffer.rows[0].chars, "42") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[countIdx] == HL_NUMBER);

    /* YAML */
    resetTestBuffer();
    currentFile = strdup("config.yaml");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "YAML") == 0);

    setRowContent(0, "version: 2.2 # yaml version");
    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_KEYWORD); /* version */
    int yNumIdx = (int)(strstr(buffer.rows[0].chars, "2.2") - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[yNumIdx] == HL_NUMBER);
    int yCmtIdx = (int)(strchr(buffer.rows[0].chars, '#') - buffer.rows[0].chars);
    assert(buffer.rows[0].hl[yCmtIdx] == HL_COMMENT);

    /* Markdown */
    resetTestBuffer();
    currentFile = strdup("docs.md");
    highlightInit();
    assert(strcmp(highlightLanguageName(), "Markdown") == 0);

    setRowContent(0, "# Title Heading");
    setRowContent(1, "```c");
    setRowContent(2, "int x = 1;");
    setRowContent(3, "```");

    highlightRow(&buffer.rows[0], 0);
    assert(buffer.rows[0].hl[0] == HL_KEYWORD); /* # Heading */

    highlightRow(&buffer.rows[1], 1);
    assert(buffer.rows[1].hl[0] == HL_PREPROCESSOR); /* ``` code block */
    assert(buffer.rows[1].hlOpenState == HL_STATE_MD_CODEBLOCK);

    highlightRow(&buffer.rows[2], 2);
    assert(buffer.rows[2].hl[0] == HL_STRING); /* Code inside block */
    assert(buffer.rows[2].hlOpenState == HL_STATE_MD_CODEBLOCK);

    highlightRow(&buffer.rows[3], 3);
    assert(buffer.rows[3].hlOpenState == HL_STATE_NONE);

    /* Markdown Links and Bold */
    setRowContent(4, "Check [Claw Editor](https://github.com/claw) now.");
    setRowContent(5, "This is **very important** text.");

    highlightRow(&buffer.rows[4], 4);
    int linkBracket = (int)(strchr(buffer.rows[4].chars, '[') - buffer.rows[4].chars);
    assert(buffer.rows[4].hl[linkBracket] == HL_OPERATOR);
    int linkText = (int)(strstr(buffer.rows[4].chars, "Claw Editor") - buffer.rows[4].chars);
    assert(buffer.rows[4].hl[linkText] == HL_TYPE);
    int urlText = (int)(strstr(buffer.rows[4].chars, "https://github.com/claw") - buffer.rows[4].chars);
    assert(buffer.rows[4].hl[urlText] == HL_STRING);

    highlightRow(&buffer.rows[5], 5);
    int boldIdx = (int)(strstr(buffer.rows[5].chars, "**very important**") - buffer.rows[5].chars);
    assert(buffer.rows[5].hl[boldIdx] == HL_KEYWORD);
    assert(buffer.rows[5].hl[boldIdx + 10] == HL_KEYWORD);

    printf("  -> Structured Data Languages PASSED.\n");
}

/* ── Test Suite 7: Language-Aware Comment Toggling ──────────────── */

static void test_comment_toggling(void)
{
    printf("[7/10] Testing Language-Aware Comment Toggling...\n");

    /* 1. Python comment toggling with '#' */
    resetTestBuffer();
    currentFile = strdup("test.py");
    highlightInit();
    assert(strcmp(highlightGetCommentPrefix(), "# ") == 0);

    setRowContent(0, "    print(\"Hello Python\")");
    cursor.x = 4;
    cursor.y = 0;

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "    # print(\"Hello Python\")") == 0);

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "    print(\"Hello Python\")") == 0);

    /* 2. C comment toggling with '//' */
    resetTestBuffer();
    currentFile = strdup("main.c");
    highlightInit();
    assert(strcmp(highlightGetCommentPrefix(), "// ") == 0);

    setRowContent(0, "    int count = 10;");
    cursor.x = 4;
    cursor.y = 0;

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "    // int count = 10;") == 0);

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "    int count = 10;") == 0);

    /* 3. Markdown comment toggling with '<!--' and '-->' */
    resetTestBuffer();
    currentFile = strdup("README.md");
    highlightInit();
    assert(strcmp(highlightGetCommentPrefix(), "<!-- ") == 0);
    assert(strcmp(highlightGetCommentSuffix(), " -->") == 0);

    setRowContent(0, "Section Title");
    cursor.x = 0;
    cursor.y = 0;

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "<!-- Section Title -->") == 0);

    toggleCommentLine();
    assert(strcmp(buffer.rows[0].chars, "Section Title") == 0);

    printf("  -> Language-Aware Comment Toggling PASSED.\n");
}

/* ── Test Suite 8: Language-Aware Smart Indentation ─────────────── */

static void test_smart_indentation(void)
{
    printf("[8/10] Testing Language-Aware Smart Indentation on Enter...\n");

    configInit();
    editorConfig.tabWidth = 4;

    /* 1. Python colon trigger ':' */
    resetTestBuffer();
    currentFile = strdup("logic.py");
    highlightInit();
    assert(highlightGetIndentTrigger() == ':');

    setRowContent(0, "def calculate():");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    /* Row 1 should have 4 leading spaces */
    assert(cursor.y == 1);
    assert(cursor.x == 4);
    assert(buffer.rows[1].size == 4);
    assert(strncmp(buffer.rows[1].chars, "    ", 4) == 0);

    /* 2. C curly brace trigger '{' */
    resetTestBuffer();
    currentFile = strdup("test.c");
    highlightInit();
    assert(highlightGetIndentTrigger() == '{');

    setRowContent(0, "void run(void) {");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    assert(cursor.y == 1);
    assert(cursor.x == 4);
    assert(buffer.rows[1].size == 4);
    assert(strncmp(buffer.rows[1].chars, "    ", 4) == 0);

    /* 3. Non-trigger statement retains base indentation without extra tab */
    resetTestBuffer();
    currentFile = strdup("test.c");
    highlightInit();

    setRowContent(0, "    int x = 42;");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    assert(cursor.y == 1);
    /* 4. Python line ending in comment with colon should NOT trigger indentation */
    resetTestBuffer();
    currentFile = strdup("test.py");
    highlightInit();

    setRowContent(0, "x = 42 # step 1:");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    assert(cursor.y == 1);
    assert(cursor.x == 0); /* No extra indent triggered by comment */

    /* 5. Python line ending in colon before a comment SHOULD trigger indentation */
    resetTestBuffer();
    currentFile = strdup("test.py");
    highlightInit();

    setRowContent(0, "def compute(): # function compute");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    assert(cursor.y == 1);
    assert(cursor.x == 4); /* Smart indent triggered despite trailing comment */

    /* 6. Markdown with brackets should NOT trigger smart indentation */
    resetTestBuffer();
    currentFile = strdup("notes.md");
    highlightInit();
    assert(highlightGetIndentTrigger() == 0);

    setRowContent(0, "[Section Link](");
    cursor.y = 0;
    cursor.x = buffer.rows[0].size;

    insertNewLine();
    assert(buffer.numRows == 2);
    assert(cursor.y == 1);
    assert(cursor.x == 0); /* No smart indent in Markdown */

    printf("  -> Language-Aware Smart Indentation PASSED.\n");
}

/* ── Test Suite 9: Buffer Manager & Multi-Language Switching ────── */

static void test_buffer_manager_language_integration(void)
{
    printf("[9/10] Testing Buffer Manager Dynamic Language Integration...\n");

    bufferManagerInit();
    assert(bufferManagerCount() >= 1);

    /* Open three different language files */
    int pyBuf = bufferManagerOpen("script.py");
    assert(pyBuf >= 0);
    assert(strcmp(bufferManagerGetLanguage(pyBuf), "Python") == 0);
    assert(strcmp(highlightLanguageName(), "Python") == 0);

    int rsBuf = bufferManagerOpen("main.rs");
    assert(rsBuf >= 0);
    assert(strcmp(bufferManagerGetLanguage(rsBuf), "Rust") == 0);
    assert(strcmp(highlightLanguageName(), "Rust") == 0);

    int cBuf = bufferManagerOpen("core.c");
    assert(cBuf >= 0);
    assert(strcmp(bufferManagerGetLanguage(cBuf), "C") == 0);
    assert(strcmp(highlightLanguageName(), "C") == 0);

    /* Switch between buffers and ensure highlightInit updates active language */
    bufferManagerSwitchTo(pyBuf);
    assert(strcmp(highlightLanguageName(), "Python") == 0);
    assert(highlightGetIndentTrigger() == ':');
    assert(strcmp(highlightGetCommentPrefix(), "# ") == 0);

    bufferManagerSwitchTo(rsBuf);
    assert(strcmp(highlightLanguageName(), "Rust") == 0);
    assert(highlightGetIndentTrigger() == '{');
    assert(strcmp(highlightGetCommentPrefix(), "// ") == 0);

    bufferManagerSwitchTo(cBuf);
    assert(strcmp(highlightLanguageName(), "C") == 0);

    bufferManagerFreeAll();
    printf("  -> Buffer Manager Dynamic Language Integration PASSED.\n");
}

/* ── Test Suite 10: Large Buffer & Iterative Non-Recursive Continuation ── */

static void test_large_buffer_continuation(void)
{
    printf("[10/10] Testing Large Buffer Iterative Continuation (Stack Safety)...\n");

    resetTestBuffer();
    currentFile = strdup("large.c");
    highlightInit();

    /* Build 2,000 rows without highlighting */
    for (int i = 0; i < 2000; i++)
    {
        if (i == 100)
            setRowContent(i, "/* Start multiline comment");
        else if (i == 500)
            setRowContent(i, "End of comment */");
        else
            setRowContent(i, "int count = 10;");
    }

    /* Highlight deep row directly - must not crash or stack-overflow */
    highlightRow(&buffer.rows[1999], 1999);
    assert(buffer.rows[1999].hl != NULL);
    assert(buffer.rows[1999].hl[0] == HL_TYPE); /* "int" */
    assert(buffer.rows[1999].hlOpenState == HL_STATE_NONE);

    /* Verify row inside the multiline comment (e.g. row 250) */
    assert(buffer.rows[250].hl != NULL);
    assert(buffer.rows[250].hl[0] == HL_MULTI_COMMENT);
    assert(buffer.rows[250].hlOpenState == HL_STATE_BLOCK_COMMENT);

    printf("  -> Large Buffer Iterative Continuation PASSED.\n");
}

/* ── Main Runner ────────────────────────────────────────────────── */

int main(void)
{
    printf("\n==================================================\n");
    printf("     Claw Phase 10 Test Suite (Language Ecosystem)\n");
    printf("==================================================\n");

    test_language_registry_and_detection();
    test_python_highlighting();
    test_js_ts_highlighting();
    test_rust_highlighting();
    test_go_and_java_highlighting();
    test_structured_data_languages();
    test_comment_toggling();
    test_smart_indentation();
    test_buffer_manager_language_integration();
    test_large_buffer_continuation();

    resetTestBuffer();

    printf("\n==================================================\n");
    printf("   ALL PHASE 10 TESTS COMPLETED & PASSED!\n");
    printf("==================================================\n\n");

    return 0;
}
