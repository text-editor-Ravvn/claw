#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

#include "highlight.h"
#include "editor.h"
#include "buffer.h"
#include "theme.h"
#include "utils.h"
#include "buffer_manager.h"

extern Buffer buffer;
extern char *currentFile;

/* Language-aware smart editing hooks declared in row.c */
extern const char *(*clawCommentPrefixHook)(void);
extern const char *(*clawCommentSuffixHook)(void);
extern char (*clawIndentTriggerHook)(void);

/* ── C language definition ──────────────────────────────────────── */

static const char *C_extensions[] = {".c", ".h", NULL};

static const char *C_keywords[] = {
    "if", "else", "while", "for", "do", "switch", "case", "default",
    "break", "continue", "return", "goto", "struct", "union", "enum",
    "typedef", "sizeof", "static", "extern", "const", "volatile",
    "register", "inline", "restrict", "_Bool", "_Complex", "_Imaginary",
    NULL};

static const char *C_types[] = {
    "int", "char", "float", "double", "void", "long", "short",
    "unsigned", "signed", "size_t", "ssize_t", "intptr_t", "uintptr_t",
    "FILE", "NULL", "int8_t", "int16_t", "int32_t", "int64_t",
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
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_NONE,
};

/* ── C++ language definition ────────────────────────────────────── */

static const char *CPP_extensions[] = {".cpp", ".cc", ".cxx", ".hpp", ".hxx", ".hh", NULL};

static const char *CPP_keywords[] = {
    "if", "else", "while", "for", "do", "switch", "case", "default",
    "break", "continue", "return", "goto", "struct", "union", "enum",
    "typedef", "sizeof", "static", "extern", "const", "volatile",
    "register", "inline", "restrict", "class", "public", "private",
    "protected", "virtual", "override", "final", "template", "typename",
    "namespace", "using", "new", "delete", "this", "friend", "operator",
    "throw", "try", "catch", "constexpr", "consteval", "decltype", "auto",
    "explicit", "mutable", "noexcept", "static_cast", "dynamic_cast",
    "reinterpret_cast", "const_cast", "concept", "requires",
    "co_await", "co_yield", "co_return",
    NULL};

static const char *CPP_types[] = {
    "int", "char", "float", "double", "void", "long", "short",
    "unsigned", "signed", "size_t", "ssize_t", "intptr_t", "uintptr_t",
    "FILE", "NULL", "nullptr", "nullptr_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "bool", "true", "false", "string", "vector", "map", "unordered_map",
    "set", "unordered_set", "pair", "unique_ptr", "shared_ptr", "weak_ptr",
    NULL};

static SyntaxDef CPP_syntax = {
    .name = "C++",
    .extensions = CPP_extensions,
    .keywords = CPP_keywords,
    .types = CPP_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = '#',
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_NONE,
};

/* ── Python language definition ─────────────────────────────────── */

static const char *Python_extensions[] = {".py", ".pyw", ".pyi", NULL};

static const char *Python_keywords[] = {
    "and", "as", "assert", "async", "await", "break", "case", "class",
    "continue", "def", "del", "elif", "else", "except", "finally",
    "for", "from", "global", "if", "import", "in", "is", "lambda",
    "match", "nonlocal", "not", "or", "pass", "raise", "return",
    "try", "while", "with", "yield",
    NULL};

static const char *Python_types[] = {
    "int", "float", "str", "bool", "list", "dict", "set", "tuple",
    "bytes", "bytearray", "None", "True", "False", "self", "cls",
    "print", "len", "range", "enumerate", "zip", "isinstance", "issubclass",
    "type", "open", "super", "any", "all", "sum", "min", "max",
    "abs", "round", "pow", "repr", "input", "id", "iter", "next",
    "reversed", "sorted", "hash", "map", "filter", "object",
    "Exception", "ValueError", "TypeError", "KeyError", "IndexError",
    "AttributeError", "IOError", "FileNotFoundError", "StopIteration",
    "RuntimeError",
    NULL};

static SyntaxDef Python_syntax = {
    .name = "Python",
    .extensions = Python_extensions,
    .keywords = Python_keywords,
    .types = Python_types,
    .singleLineComment = "#",
    .multiLineCommentStart = "\"\"\"",
    .multiLineCommentEnd = "\"\"\"",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "# ",
    .commentSuffix = NULL,
    .indentTrigger = ':',
    .flags = LANG_FLAG_DOCSTRINGS,
};

/* ── JavaScript language definition ─────────────────────────────── */

static const char *JS_extensions[] = {".js", ".mjs", ".cjs", ".jsx", NULL};

static const char *JS_keywords[] = {
    "async", "await", "break", "case", "catch", "class", "const",
    "continue", "debugger", "default", "delete", "do", "else", "export",
    "extends", "finally", "for", "function", "from", "if", "import",
    "in", "instanceof", "let", "new", "of", "return", "static",
    "super", "switch", "this", "throw", "try", "typeof", "var",
    "void", "while", "with", "yield", "as",
    NULL};

static const char *JS_types[] = {
    "true", "false", "null", "undefined", "NaN", "Infinity",
    "number", "string", "boolean", "symbol", "bigint",
    "Array", "Object", "Function", "Promise", "Map", "Set",
    "WeakMap", "WeakSet", "Date", "RegExp", "Error", "Math",
    "JSON", "console", "document", "window",
    NULL};

static SyntaxDef JS_syntax = {
    .name = "JavaScript",
    .extensions = JS_extensions,
    .keywords = JS_keywords,
    .types = JS_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_TEMPLATES | LANG_FLAG_JS_REGEX,
};

/* ── TypeScript language definition ─────────────────────────────── */

static const char *TS_extensions[] = {".ts", ".tsx", ".mts", ".cts", NULL};

static const char *TS_keywords[] = {
    "abstract", "any", "as", "async", "await", "break", "case", "catch",
    "class", "const", "continue", "debugger", "declare", "default", "delete",
    "do", "else", "enum", "export", "extends", "finally", "for", "from",
    "function", "if", "implements", "import", "in", "instanceof", "interface",
    "is", "keyof", "let", "namespace", "never", "new", "of", "override",
    "private", "protected", "public", "readonly", "return", "satisfies",
    "static", "super", "switch", "this", "throw", "try", "type", "typeof",
    "unknown", "var", "void", "while", "with", "yield",
    NULL};

static const char *TS_types[] = {
    "true", "false", "null", "undefined", "NaN", "Infinity",
    "number", "string", "boolean", "symbol", "bigint", "void",
    "Array", "Object", "Function", "Promise", "Map", "Set",
    "WeakMap", "WeakSet", "Date", "RegExp", "Error", "Math", "JSON",
    "console", "document", "window",
    "Record", "Partial", "Required", "Readonly", "Pick", "Omit",
    "Exclude", "Extract", "NonNullable", "Parameters", "ReturnType",
    "InstanceType",
    NULL};

static SyntaxDef TS_syntax = {
    .name = "TypeScript",
    .extensions = TS_extensions,
    .keywords = TS_keywords,
    .types = TS_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_TEMPLATES | LANG_FLAG_JS_REGEX,
};

/* ── Rust language definition ───────────────────────────────────── */

static const char *Rust_extensions[] = {".rs", NULL};

static const char *Rust_keywords[] = {
    "as", "async", "await", "break", "const", "continue", "crate",
    "dyn", "else", "enum", "extern", "false", "fn", "for", "if",
    "impl", "in", "let", "loop", "match", "mod", "move", "mut",
    "pub", "ref", "return", "self", "Self", "static", "struct",
    "super", "trait", "true", "type", "unsafe", "use", "where",
    "while",
    NULL};

static const char *Rust_types[] = {
    "i8", "i16", "i32", "i64", "i128", "isize",
    "u8", "u16", "u32", "u64", "u128", "usize",
    "f32", "f64", "bool", "char", "str", "String",
    "Option", "Some", "None", "Result", "Ok", "Err",
    "Vec", "Box", "Rc", "Arc", "Cell", "RefCell",
    NULL};

static SyntaxDef Rust_syntax = {
    .name = "Rust",
    .extensions = Rust_extensions,
    .keywords = Rust_keywords,
    .types = Rust_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_RUST_MACROS | LANG_FLAG_RUST_LIFETIME,
};

/* ── Go language definition ─────────────────────────────────────── */

static const char *Go_extensions[] = {".go", NULL};

static const char *Go_keywords[] = {
    "break", "case", "chan", "const", "continue", "default", "defer",
    "else", "fallthrough", "for", "func", "go", "goto", "if", "import",
    "interface", "map", "package", "range", "return", "select",
    "struct", "switch", "type", "var",
    NULL};

static const char *Go_types[] = {
    "int", "int8", "int16", "int32", "int64",
    "uint", "uint8", "uint16", "uint32", "uint64", "uintptr",
    "float32", "float64", "complex64", "complex128",
    "byte", "rune", "string", "bool", "error",
    "true", "false", "iota", "nil",
    "make", "len", "cap", "new", "append", "copy", "close", "delete",
    "panic", "recover", "print", "println",
    NULL};

static SyntaxDef Go_syntax = {
    .name = "Go",
    .extensions = Go_extensions,
    .keywords = Go_keywords,
    .types = Go_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_TEMPLATES,
};

/* ── Java language definition ───────────────────────────────────── */

static const char *Java_extensions[] = {".java", NULL};

static const char *Java_keywords[] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch",
    "char", "class", "const", "continue", "default", "do", "double",
    "else", "enum", "extends", "final", "finally", "float", "for",
    "goto", "if", "implements", "import", "instanceof", "int", "interface",
    "long", "native", "new", "package", "permits", "private", "protected",
    "public", "record", "return", "sealed", "short", "static", "strictfp",
    "super", "switch", "synchronized", "this", "throw", "throws",
    "transient", "try", "var", "void", "volatile", "while", "yield",
    NULL};

static const char *Java_types[] = {
    "String", "Object", "Integer", "Long", "Float", "Double",
    "Boolean", "Character", "Byte", "Short", "List", "Map", "Set",
    "ArrayList", "HashMap", "HashSet", "System", "Math",
    "Exception", "Override", "true", "false", "null",
    NULL};

static SyntaxDef Java_syntax = {
    .name = "Java",
    .extensions = Java_extensions,
    .keywords = Java_keywords,
    .types = Java_types,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_NONE,
};

/* ── JSON language definition ───────────────────────────────────── */

static const char *JSON_extensions[] = {".json", NULL};

static const char *JSON_keywords[] = {
    "true", "false", "null",
    NULL};

static SyntaxDef JSON_syntax = {
    .name = "JSON",
    .extensions = JSON_extensions,
    .keywords = JSON_keywords,
    .types = NULL,
    .singleLineComment = "//",
    .multiLineCommentStart = "/*",
    .multiLineCommentEnd = "*/",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "// ",
    .commentSuffix = NULL,
    .indentTrigger = '{',
    .flags = LANG_FLAG_NONE,
};

/* ── YAML language definition ───────────────────────────────────── */

static const char *YAML_extensions[] = {".yaml", ".yml", NULL};

static const char *YAML_keywords[] = {
    "true", "false", "null", "yes", "no", "on", "off", "~",
    NULL};

static SyntaxDef YAML_syntax = {
    .name = "YAML",
    .extensions = YAML_extensions,
    .keywords = YAML_keywords,
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
    .flags = LANG_FLAG_YAML,
};

/* ── Markdown language definition ───────────────────────────────── */

static const char *Markdown_extensions[] = {".md", ".markdown", NULL};

static SyntaxDef Markdown_syntax = {
    .name = "Markdown",
    .extensions = Markdown_extensions,
    .keywords = NULL,
    .types = NULL,
    .singleLineComment = NULL,
    .multiLineCommentStart = "<!--",
    .multiLineCommentEnd = "-->",
    .preprocessorChar = 0,
    .highlightStrings = 0,
    .highlightNumbers = 0,
    .commentPrefix = "<!-- ",
    .commentSuffix = " -->",
    .indentTrigger = 0,
    .flags = LANG_FLAG_MARKDOWN,
};

/* ── Shell / Bash language definition ───────────────────────────── */

static const char *Shell_extensions[] = {".sh", ".bash", ".zsh", NULL};

static const char *Shell_keywords[] = {
    "if", "then", "else", "elif", "fi", "case", "esac", "for",
    "select", "while", "until", "do", "done", "in", "function",
    "time", "echo", "exit", "return", "export", "local", "unset",
    "readonly", "set", "shift", "source",
    NULL};

static SyntaxDef Shell_syntax = {
    .name = "Shell",
    .extensions = Shell_extensions,
    .keywords = Shell_keywords,
    .types = NULL,
    .singleLineComment = "#",
    .multiLineCommentStart = NULL,
    .multiLineCommentEnd = NULL,
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 1,
    .commentPrefix = "# ",
    .commentSuffix = NULL,
    .indentTrigger = 0,
    .flags = LANG_FLAG_NONE,
};

/* ── HTML / XML language definition ─────────────────────────────── */

static const char *HTML_extensions[] = {".html", ".htm", ".xml", NULL};

static const char *HTML_keywords[] = {
    "DOCTYPE", "doctype", "html", "head", "body", "div", "span", "script", "style",
    NULL};

static SyntaxDef HTML_syntax = {
    .name = "HTML",
    .extensions = HTML_extensions,
    .keywords = HTML_keywords,
    .types = NULL,
    .singleLineComment = NULL,
    .multiLineCommentStart = "<!--",
    .multiLineCommentEnd = "-->",
    .preprocessorChar = 0,
    .highlightStrings = 1,
    .highlightNumbers = 0,
    .commentPrefix = "<!-- ",
    .commentSuffix = " -->",
    .indentTrigger = 0,
    .flags = LANG_FLAG_NONE,
};

/* ── Language Registry ──────────────────────────────────────────── */

#define MAX_LANGUAGES 64
static SyntaxDef *LANGUAGES[MAX_LANGUAGES];
static int numLanguages = 0;
static SyntaxDef *activeSyntax = NULL;
static int registryInitialized = 0;

static void initLanguageRegistry(void)
{
    if (registryInitialized)
        return;

    numLanguages = 0;
    LANGUAGES[numLanguages++] = &C_syntax;
    LANGUAGES[numLanguages++] = &CPP_syntax;
    LANGUAGES[numLanguages++] = &Python_syntax;
    LANGUAGES[numLanguages++] = &JS_syntax;
    LANGUAGES[numLanguages++] = &TS_syntax;
    LANGUAGES[numLanguages++] = &Rust_syntax;
    LANGUAGES[numLanguages++] = &Go_syntax;
    LANGUAGES[numLanguages++] = &Java_syntax;
    LANGUAGES[numLanguages++] = &JSON_syntax;
    LANGUAGES[numLanguages++] = &YAML_syntax;
    LANGUAGES[numLanguages++] = &Markdown_syntax;
    LANGUAGES[numLanguages++] = &Shell_syntax;
    LANGUAGES[numLanguages++] = &HTML_syntax;
    LANGUAGES[numLanguages] = NULL;

    registryInitialized = 1;

    /* Wire up hooks into row.c */
    clawCommentPrefixHook = highlightGetCommentPrefix;
    clawCommentSuffixHook = highlightGetCommentSuffix;
    clawIndentTriggerHook = highlightGetIndentTrigger;
}

int highlightRegisterLanguage(SyntaxDef *syntax)
{
    initLanguageRegistry();
    if (!syntax || !syntax->name || numLanguages >= MAX_LANGUAGES - 1)
        return 0;

    for (int i = 0; i < numLanguages; i++)
    {
        if (strcasecmp(LANGUAGES[i]->name, syntax->name) == 0)
        {
            LANGUAGES[i] = syntax;
            return 1;
        }
    }

    LANGUAGES[numLanguages++] = syntax;
    LANGUAGES[numLanguages] = NULL;
    return 1;
}

int highlightLanguageCount(void)
{
    initLanguageRegistry();
    return numLanguages;
}

const SyntaxDef *highlightGetLanguage(int index)
{
    initLanguageRegistry();
    if (index < 0 || index >= numLanguages)
        return NULL;
    return LANGUAGES[index];
}

const SyntaxDef *highlightFindLanguageByName(const char *name)
{
    initLanguageRegistry();
    if (!name || !*name)
        return NULL;

    for (int i = 0; i < numLanguages; i++)
    {
        if (strcasecmp(LANGUAGES[i]->name, name) == 0)
            return LANGUAGES[i];
    }
    return NULL;
}

const SyntaxDef *highlightFindLanguageByFilename(const char *filename)
{
    initLanguageRegistry();
    if (!filename || !*filename)
        return NULL;

    /* Check special filenames */
    const char *lastSlash = strrchr(filename, '/');
    const char *lastBackslash = strrchr(filename, '\\');
    const char *base = filename;
    if (lastSlash && lastSlash > base)
        base = lastSlash + 1;
    if (lastBackslash && lastBackslash > base)
        base = lastBackslash + 1;

    if (strcasecmp(base, "Makefile") == 0 ||
        strcasecmp(base, "Dockerfile") == 0)
    {
        return highlightFindLanguageByName("Shell");
    }

    /* Find the last dot in the filename */
    const char *dot = strrchr(base, '.');
    if (!dot)
        return NULL;

    for (int i = 0; i < numLanguages; i++)
    {
        const char **ext = LANGUAGES[i]->extensions;
        if (!ext)
            continue;

        for (int j = 0; ext[j] != NULL; j++)
        {
            if (strcasecmp(dot, ext[j]) == 0)
                return LANGUAGES[i];
        }
    }
    return NULL;
}

const SyntaxDef *highlightDetectLanguageFromContent(const char *firstLine)
{
    initLanguageRegistry();
    if (!firstLine || !*firstLine)
        return NULL;

    if (firstLine[0] == '#' && firstLine[1] == '!')
    {
        if (strstr(firstLine, "python"))
            return highlightFindLanguageByName("Python");
        if (strstr(firstLine, "node"))
            return highlightFindLanguageByName("JavaScript");
        if (strstr(firstLine, "bash") || strstr(firstLine, "sh") || strstr(firstLine, "zsh"))
            return highlightFindLanguageByName("Shell");
        if (strstr(firstLine, "perl"))
            return highlightFindLanguageByName("Shell");
    }
    return NULL;
}

int highlightSetLanguage(const char *name)
{
    initLanguageRegistry();
    if (!name || !*name || strcasecmp(name, "Plain Text") == 0 || strcasecmp(name, "plain") == 0)
    {
        activeSyntax = NULL;
        return 1;
    }

    const SyntaxDef *syn = highlightFindLanguageByName(name);
    if (syn)
    {
        activeSyntax = (SyntaxDef *)syn;
        return 1;
    }
    return 0;
}

const SyntaxDef *highlightGetActiveSyntax(void)
{
    initLanguageRegistry();
    return activeSyntax;
}

const char *highlightGetCommentPrefix(void)
{
    initLanguageRegistry();
    if (activeSyntax && activeSyntax->commentPrefix)
        return activeSyntax->commentPrefix;
    if (activeSyntax && activeSyntax->singleLineComment)
        return activeSyntax->singleLineComment;
    return "// ";
}

const char *highlightGetCommentSuffix(void)
{
    initLanguageRegistry();
    if (activeSyntax && activeSyntax->commentSuffix)
        return activeSyntax->commentSuffix;
    return NULL;
}

char highlightGetIndentTrigger(void)
{
    initLanguageRegistry();
    if (activeSyntax)
        return activeSyntax->indentTrigger;
    return 0;
}

/* ── Helpers ────────────────────────────────────────────────────── */

static int isSeparator(int c)
{
    return isspace(c) || c == '\0' ||
           strchr(",.()+-/*=~%<>[];:{}&|^!?#\"'`", c) != NULL;
}

/* ── Public API ─────────────────────────────────────────────────── */

void highlightInit(void)
{
    initLanguageRegistry();
    activeSyntax = NULL;

    if (currentFile && currentFile[0] != '\0')
    {
        const SyntaxDef *syn = highlightFindLanguageByFilename(currentFile);
        if (syn)
        {
            activeSyntax = (SyntaxDef *)syn;
            return;
        }
    }

    /* Check shebang in first line if available */
    if (buffer.numRows > 0 && buffer.rows && buffer.rows[0].chars)
    {
        const SyntaxDef *syn = highlightDetectLanguageFromContent(buffer.rows[0].chars);
        if (syn)
        {
            activeSyntax = (SyntaxDef *)syn;
            return;
        }
    }
}

const char *highlightLanguageName(void)
{
    initLanguageRegistry();
    if (activeSyntax)
        return activeSyntax->name;

    return "Plain Text";
}

const char *highlightColor(int hlType)
{
    return themeColor(hlType);
}

static int isEscaped(const char *chars, int pos)
{
    int backslashes = 0;
    while (pos > 0 && chars[pos - 1] == '\\')
    {
        backslashes++;
        pos--;
    }
    return (backslashes % 2) != 0;
}

static int canStartJsRegex(const char *chars, int pos)
{
    int k = pos - 1;
    while (k >= 0 && isspace((unsigned char)chars[k]))
        k--;

    if (k < 0)
        return 1;

    char prev = chars[k];
    if (strchr("=+-*%/&|^!~<>?:([{},;", prev))
        return 1;

    if (isalnum((unsigned char)prev) || prev == '_' || prev == '$')
    {
        int end = k;
        while (k >= 0 && (isalnum((unsigned char)chars[k]) || chars[k] == '_' || chars[k] == '$'))
            k--;
        int start = k + 1;
        int wordLen = end - start + 1;
        static const char *exprKws[] = {
            "return", "yield", "case", "await", "throw", "typeof", "delete", "void", "in", "instanceof", "of", NULL
        };
        for (int w = 0; exprKws[w]; w++)
        {
            if ((int)strlen(exprKws[w]) == wordLen && memcmp(&chars[start], exprKws[w], wordLen) == 0)
                return 1;
        }
    }

    return 0;
}

static void highlightRowProcess(Row *row, const SyntaxDef *syntax, int inState)
{
    if (!syntax)
    {
        row->hlOpenState = HL_STATE_NONE;
        return;
    }

    char *chars = row->chars;
    int size = row->size;
    const char *scs = syntax->singleLineComment;
    const char *mcs = syntax->multiLineCommentStart;
    const char *mce = syntax->multiLineCommentEnd;
    int scsLen = scs ? (int)strlen(scs) : 0;
    int mcsLen = mcs ? (int)strlen(mcs) : 0;
    int mceLen = mce ? (int)strlen(mce) : 0;
    int prevSep = 1;
    int i = 0;

    /* If row is empty, preserve continuation state */
    if (size == 0)
    {
        row->hlOpenState = inState;
        return;
    }

    /* ── Continuation state handling ── */
    if (inState == HL_STATE_MD_CODEBLOCK)
    {
        int j = 0;
        while (j < size && isspace((unsigned char)chars[j]))
            j++;
        if (j + 3 <= size && memcmp(&chars[j], "```", 3) == 0)
        {
            memset(row->hl, HL_PREPROCESSOR, size);
            row->hlOpenState = HL_STATE_NONE;
            return;
        }
        memset(row->hl, HL_STRING, size);
        row->hlOpenState = HL_STATE_MD_CODEBLOCK;
        return;
    }

    while (i < size && inState != HL_STATE_NONE)
    {
        if (inState == HL_STATE_BLOCK_COMMENT)
        {
            row->hl[i] = HL_MULTI_COMMENT;
            if (mceLen > 0 && i + mceLen <= size && memcmp(&chars[i], mce, mceLen) == 0)
            {
                memset(&row->hl[i], HL_MULTI_COMMENT, mceLen);
                i += mceLen;
                inState = HL_STATE_NONE;
                prevSep = 1;
                break;
            }
            i++;
        }
        else if (inState == HL_STATE_TRIPLE_DOUBLE)
        {
            row->hl[i] = HL_STRING;
            if (!isEscaped(chars, i) && i + 3 <= size && memcmp(&chars[i], "\"\"\"", 3) == 0)
            {
                memset(&row->hl[i], HL_STRING, 3);
                i += 3;
                inState = HL_STATE_NONE;
                prevSep = 1;
                break;
            }
            i++;
        }
        else if (inState == HL_STATE_TRIPLE_SINGLE)
        {
            row->hl[i] = HL_STRING;
            if (!isEscaped(chars, i) && i + 3 <= size && memcmp(&chars[i], "'''", 3) == 0)
            {
                memset(&row->hl[i], HL_STRING, 3);
                i += 3;
                inState = HL_STATE_NONE;
                prevSep = 1;
                break;
            }
            i++;
        }
        else if (inState == HL_STATE_BACKTICK)
        {
            row->hl[i] = HL_STRING;
            if (chars[i] == '\\' && i + 1 < size)
            {
                row->hl[i + 1] = HL_STRING;
                i += 2;
                continue;
            }
            if (chars[i] == '`')
            {
                i++;
                inState = HL_STATE_NONE;
                prevSep = 1;
                break;
            }
            i++;
        }
        else
        {
            inState = HL_STATE_NONE;
            break;
        }
    }

    /* ── Markdown document handling ── */
    if ((syntax->flags & LANG_FLAG_MARKDOWN) && inState == HL_STATE_NONE && i == 0)
    {
        int j = 0;
        while (j < size && isspace((unsigned char)chars[j]))
            j++;

        /* Code block open */
        if (j + 3 <= size && memcmp(&chars[j], "```", 3) == 0)
        {
            memset(row->hl, HL_PREPROCESSOR, size);
            row->hlOpenState = HL_STATE_MD_CODEBLOCK;
            return;
        }

        /* Header line */
        if (j < size && chars[j] == '#')
        {
            memset(row->hl, HL_KEYWORD, size);
            row->hlOpenState = HL_STATE_NONE;
            return;
        }
    }

    /* ── Preprocessor line check ── */
    if (syntax->preprocessorChar && inState == HL_STATE_NONE && i == 0)
    {
        int j = 0;
        while (j < size && isspace((unsigned char)chars[j]))
            j++;
        if (j < size && chars[j] == syntax->preprocessorChar)
        {
            memset(row->hl, HL_PREPROCESSOR, size);
            row->hlOpenState = HL_STATE_NONE;
            return;
        }
    }

    /* ── Main syntax scan loop ── */
    while (i < size)
    {
        /* Python docstrings """ or ''' */
        if ((syntax->flags & LANG_FLAG_DOCSTRINGS) &&
            i + 3 <= size &&
            (memcmp(&chars[i], "\"\"\"", 3) == 0 || memcmp(&chars[i], "'''", 3) == 0))
        {
            const char quoteChar = chars[i];
            const char *quoteStr = (quoteChar == '"') ? "\"\"\"" : "'''";
            memset(&row->hl[i], HL_STRING, 3);
            i += 3;

            int closed = 0;
            while (i + 3 <= size)
            {
                if (!isEscaped(chars, i) && memcmp(&chars[i], quoteStr, 3) == 0)
                {
                    memset(&row->hl[i], HL_STRING, 3);
                    i += 3;
                    closed = 1;
                    break;
                }
                row->hl[i] = HL_STRING;
                i++;
            }

            if (!closed)
            {
                while (i < size)
                {
                    row->hl[i] = HL_STRING;
                    i++;
                }
                inState = (quoteChar == '"') ? HL_STATE_TRIPLE_DOUBLE : HL_STATE_TRIPLE_SINGLE;
                row->hlOpenState = inState;
                return;
            }
            prevSep = 1;
            continue;
        }

        /* Backtick template/raw strings (JS, TS, Go) */
        if ((syntax->flags & LANG_FLAG_TEMPLATES) && chars[i] == '`')
        {
            row->hl[i] = HL_STRING;
            i++;
            int closed = 0;
            while (i < size)
            {
                row->hl[i] = HL_STRING;
                if (chars[i] == '\\' && i + 1 < size)
                {
                    row->hl[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if (chars[i] == '`')
                {
                    i++;
                    closed = 1;
                    break;
                }
                i++;
            }
            if (!closed)
            {
                inState = HL_STATE_BACKTICK;
                row->hlOpenState = inState;
                return;
            }
            prevSep = 1;
            continue;
        }

        /* Single-line comments */
        if (scsLen > 0 && i + scsLen <= size && memcmp(&chars[i], scs, scsLen) == 0)
        {
            memset(&row->hl[i], HL_COMMENT, size - i);
            row->hlOpenState = inState;
            return;
        }

        /* Multi-line comments start */
        if (mcsLen > 0 && i + mcsLen <= size && memcmp(&chars[i], mcs, mcsLen) == 0)
        {
            memset(&row->hl[i], HL_MULTI_COMMENT, mcsLen);
            i += mcsLen;
            inState = HL_STATE_BLOCK_COMMENT;
            while (i < size)
            {
                row->hl[i] = HL_MULTI_COMMENT;
                if (mceLen > 0 && i + mceLen <= size && memcmp(&chars[i], mce, mceLen) == 0)
                {
                    memset(&row->hl[i], HL_MULTI_COMMENT, mceLen);
                    i += mceLen;
                    inState = HL_STATE_NONE;
                    break;
                }
                i++;
            }
            prevSep = 1;
            continue;
        }

        /* JS/TS Regex Literals: /pattern/flags */
        if ((syntax->flags & LANG_FLAG_JS_REGEX) && chars[i] == '/')
        {
            if (canStartJsRegex(chars, i))
            {
                int r = i + 1;
                int inClass = 0;
                int foundEnd = 0;
                while (r < size)
                {
                    if (chars[r] == '\\' && r + 1 < size)
                    {
                        r += 2;
                        continue;
                    }
                    if (chars[r] == '[')
                        inClass = 1;
                    else if (chars[r] == ']')
                        inClass = 0;
                    else if (chars[r] == '/' && !inClass)
                    {
                        foundEnd = 1;
                        r++;
                        break;
                    }
                    r++;
                }
                if (foundEnd)
                {
                    while (r < size && isalpha((unsigned char)chars[r]))
                        r++;
                    memset(&row->hl[i], HL_STRING, r - i);
                    i = r;
                    prevSep = 1;
                    continue;
                }
            }
        }

        /* Rust attributes #[...] or #![...] */
        if ((syntax->flags & LANG_FLAG_RUST_MACROS) &&
            chars[i] == '#' && i + 1 < size &&
            (chars[i + 1] == '[' || (chars[i + 1] == '!' && i + 2 < size && chars[i + 2] == '[')))
        {
            int depth = 0;
            while (i < size)
            {
                row->hl[i] = HL_PREPROCESSOR;
                if (chars[i] == '[')
                    depth++;
                else if (chars[i] == ']')
                {
                    depth--;
                    if (depth <= 0)
                    {
                        i++;
                        break;
                    }
                }
                i++;
            }
            prevSep = 1;
            continue;
        }

        /* Rust lifetimes 'a, 'static vs character literals */
        if ((syntax->flags & LANG_FLAG_RUST_LIFETIME) && chars[i] == '\'')
        {
            if (i + 1 < size && (isalpha((unsigned char)chars[i + 1]) || chars[i + 1] == '_'))
            {
                int isCharLit = 0;
                if (i + 2 < size && chars[i + 2] == '\'')
                    isCharLit = 1;
                else if (chars[i + 1] == '\\' && i + 3 < size && chars[i + 3] == '\'')
                    isCharLit = 1;

                if (!isCharLit)
                {
                    row->hl[i] = HL_TYPE;
                    i++;
                    while (i < size && (isalnum((unsigned char)chars[i]) || chars[i] == '_'))
                    {
                        row->hl[i] = HL_TYPE;
                        i++;
                    }
                    prevSep = 1;
                    continue;
                }
            }
        }

        /* String and character literals */
        if (syntax->highlightStrings && (chars[i] == '"' || chars[i] == '\''))
        {
            int quote = chars[i];
            row->hl[i] = (quote == '\'') ? HL_CHAR_LITERAL : HL_STRING;
            i++;
            while (i < size)
            {
                row->hl[i] = (quote == '\'') ? HL_CHAR_LITERAL : HL_STRING;
                if (chars[i] == '\\' && i + 1 < size)
                {
                    row->hl[i + 1] = (quote == '\'') ? HL_CHAR_LITERAL : HL_STRING;
                    i += 2;
                    continue;
                }
                if (chars[i] == quote)
                {
                    i++;
                    break;
                }
                i++;
            }
            prevSep = 1;
            continue;
        }

        /* Numbers */
        if (syntax->highlightNumbers)
        {
            if ((isdigit((unsigned char)chars[i]) && prevSep) ||
                (chars[i] == '.' && i + 1 < size && isdigit((unsigned char)chars[i + 1]) && prevSep))
            {
                while (i < size &&
                       (isalnum((unsigned char)chars[i]) || chars[i] == '.' || chars[i] == '_'))
                {
                    if ((chars[i] == 'e' || chars[i] == 'E') &&
                        i + 1 < size && (chars[i + 1] == '+' || chars[i + 1] == '-'))
                    {
                        row->hl[i] = HL_NUMBER;
                        i++;
                    }
                    row->hl[i] = HL_NUMBER;
                    i++;
                }
                prevSep = 0;
                continue;
            }
        }

        /* Rust Macros: identifier followed immediately by ! (e.g. println!, vec!) */
        if ((syntax->flags & LANG_FLAG_RUST_MACROS) && prevSep &&
            (isalpha((unsigned char)chars[i]) || chars[i] == '_'))
        {
            int start = i;
            while (i < size && (isalnum((unsigned char)chars[i]) || chars[i] == '_'))
                i++;
            if (i < size && chars[i] == '!')
            {
                i++;
                memset(&row->hl[start], HL_KEYWORD, i - start);
                prevSep = 0;
                continue;
            }
            else
            {
                i = start;
            }
        }

        /* YAML key-value syntax: identifier followed by : */
        if ((syntax->flags & LANG_FLAG_YAML) && prevSep &&
            (isalpha((unsigned char)chars[i]) || chars[i] == '_'))
        {
            int start = i;
            while (i < size && (isalnum((unsigned char)chars[i]) || chars[i] == '_' || chars[i] == '-'))
                i++;
            if (i < size && chars[i] == ':')
            {
                memset(&row->hl[start], HL_KEYWORD, i - start);
                row->hl[i] = HL_OPERATOR;
                i++;
                prevSep = 1;
                continue;
            }
            else
            {
                i = start;
            }
        }

        /* Keywords and types */
        if (prevSep)
        {
            int matched = 0;

            /* Check types */
            if (syntax->types)
            {
                for (int k = 0; syntax->types[k]; k++)
                {
                    int kwLen = (int)strlen(syntax->types[k]);
                    if (i + kwLen <= size &&
                        memcmp(&chars[i], syntax->types[k], kwLen) == 0 &&
                        (i + kwLen == size || isSeparator((unsigned char)chars[i + kwLen])))
                    {
                        memset(&row->hl[i], HL_TYPE, kwLen);
                        i += kwLen;
                        prevSep = 0;
                        matched = 1;
                        break;
                    }
                }
            }

            /* Check keywords */
            if (!matched && syntax->keywords)
            {
                for (int k = 0; syntax->keywords[k]; k++)
                {
                    int kwLen = (int)strlen(syntax->keywords[k]);
                    if (i + kwLen <= size &&
                        memcmp(&chars[i], syntax->keywords[k], kwLen) == 0 &&
                        (i + kwLen == size || isSeparator((unsigned char)chars[i + kwLen])))
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

        /* Markdown blockquotes, bullets, inline code, bold, links */
        if (syntax->flags & LANG_FLAG_MARKDOWN)
        {
            /* Links: [text](url) */
            if (chars[i] == '[')
            {
                int closeBracket = i + 1;
                while (closeBracket < size && chars[closeBracket] != ']')
                    closeBracket++;
                if (closeBracket < size && chars[closeBracket] == ']' &&
                    closeBracket + 1 < size && chars[closeBracket + 1] == '(')
                {
                    int closeParen = closeBracket + 2;
                    while (closeParen < size && chars[closeParen] != ')')
                        closeParen++;
                    if (closeParen < size && chars[closeParen] == ')')
                    {
                        row->hl[i] = HL_OPERATOR;
                        for (int t = i + 1; t < closeBracket; t++)
                            row->hl[t] = HL_TYPE;
                        row->hl[closeBracket] = HL_OPERATOR;
                        row->hl[closeBracket + 1] = HL_OPERATOR;
                        for (int u = closeBracket + 2; u < closeParen; u++)
                            row->hl[u] = HL_STRING;
                        row->hl[closeParen] = HL_OPERATOR;
                        i = closeParen + 1;
                        prevSep = 1;
                        continue;
                    }
                }
            }

            /* Bold: **bold** or __bold__ */
            if (i + 1 < size &&
                ((chars[i] == '*' && chars[i + 1] == '*') ||
                 (chars[i] == '_' && chars[i + 1] == '_')))
            {
                char m = chars[i];
                int end = i + 2;
                while (end + 1 < size && !(chars[end] == m && chars[end + 1] == m))
                    end++;
                if (end + 1 < size && chars[end] == m && chars[end + 1] == m)
                {
                    memset(&row->hl[i], HL_KEYWORD, end + 2 - i);
                    i = end + 2;
                    prevSep = 1;
                    continue;
                }
            }

            /* Inline code */
            if (chars[i] == '`')
            {
                row->hl[i] = HL_STRING;
                i++;
                while (i < size && chars[i] != '`')
                {
                    row->hl[i] = HL_STRING;
                    i++;
                }
                if (i < size && chars[i] == '`')
                {
                    row->hl[i] = HL_STRING;
                    i++;
                }
                prevSep = 1;
                continue;
            }

            /* Blockquotes and bullets */
            if (chars[i] == '>' || chars[i] == '-' || chars[i] == '*' || chars[i] == '+')
            {
                row->hl[i] = HL_OPERATOR;
                i++;
                prevSep = 1;
                continue;
            }
        }

        /* Operators */
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

    row->hlOpenState = inState;
}

void highlightRow(Row *row, int rowIndex)
{
    initLanguageRegistry();

    if (!row)
        return;

    /* Ensure hl array is allocated for the row */
    int allocSize = row->size > 0 ? row->size + 1 : 1;
    if (row->hlSize < allocSize || !row->hl)
    {
        unsigned char *newHl = realloc(row->hl, allocSize);
        if (!newHl)
            return;
        row->hl = newHl;
        row->hlSize = allocSize;
    }

    memset(row->hl, HL_NORMAL, allocSize);

    /* Determine which buffer owns this row */
    Buffer *doc = NULL;
    const SyntaxDef *syn = activeSyntax;

    if (buffer.rows && rowIndex >= 0 && rowIndex < buffer.numRows && row == &buffer.rows[rowIndex])
    {
        doc = &buffer;
    }
    else
    {
        for (int b = 0; b < bufferManager.count; b++)
        {
            FileBuffer *fb = &bufferManager.buffers[b];
            if (fb->doc.rows && rowIndex >= 0 && rowIndex < fb->doc.numRows && row == &fb->doc.rows[rowIndex])
            {
                doc = &fb->doc;
                if (fb->filename[0] != '\0')
                    syn = highlightFindLanguageByFilename(fb->filename);
                if (!syn && fb->doc.numRows > 0 && fb->doc.rows && fb->doc.rows[0].chars)
                    syn = highlightDetectLanguageFromContent(fb->doc.rows[0].chars);
                break;
            }
        }
    }

    if (!syn)
    {
        row->hlOpenState = HL_STATE_NONE;
        return;
    }

    /* Determine starting continuation state iteratively without deep recursion */
    int inState = HL_STATE_NONE;
    if (doc && rowIndex > 0 && rowIndex <= doc->numRows && doc->rows)
    {
        int startRow = rowIndex - 1;
        while (startRow > 0 && !doc->rows[startRow].hl)
            startRow--;

        for (int r = startRow; r < rowIndex; r++)
        {
            if (!doc->rows[r].hl)
            {
                int rAlloc = doc->rows[r].size > 0 ? doc->rows[r].size + 1 : 1;
                doc->rows[r].hl = realloc(doc->rows[r].hl, rAlloc);
                if (doc->rows[r].hl)
                {
                    doc->rows[r].hlSize = rAlloc;
                    memset(doc->rows[r].hl, HL_NORMAL, rAlloc);
                    highlightRowProcess(&doc->rows[r], syn, inState);
                }
            }
            inState = doc->rows[r].hlOpenState;
        }
    }
    else if (!doc && rowIndex > 0 && buffer.rows && rowIndex <= buffer.numRows)
    {
        inState = buffer.rows[rowIndex - 1].hlOpenState;
    }

    highlightRowProcess(row, syn, inState);
}
