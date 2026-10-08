#include "theme.h"

static Theme defaultTheme =
{
    "\033[38;5;176m",
    "\033[38;5;80m",
    "\033[38;5;114m",
    "\033[38;5;209m",
    "\033[38;5;102m",
    "\033[38;5;179m",
    "\033[38;5;204m",
    "\033[30;43m",
    "\033[m"
};

static Theme *activeTheme =
    &defaultTheme;

void themeInit(void)
{
}

const char *themeColor(
    int hlType
)
{
    switch (hlType)
    {
        case HL_KEYWORD:
            return activeTheme->keyword;

        case HL_TYPE:
            return activeTheme->type;

        case HL_STRING:
            return activeTheme->string;

        case HL_NUMBER:
            return activeTheme->number;

        case HL_COMMENT:
        case HL_MULTI_COMMENT:
            return activeTheme->comment;

        case HL_PREPROCESSOR:
            return activeTheme->preprocessor;

        case HL_OPERATOR:
            return activeTheme->operator;

        case HL_MATCH:
            return activeTheme->match;

        default:
            return activeTheme->normal;
    }
}