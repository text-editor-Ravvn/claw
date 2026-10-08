#include "theme.h"
#include <string.h>
#include "highlight.h"

static Theme defaultTheme =
{
    "\033[38;5;176m", /* keyword */
    "\033[38;5;81m",  /* type */
    "\033[38;5;214m", /* string */
    "\033[38;5;220m", /* number */
    "\033[38;5;244m", /* comment */
    "\033[38;5;208m", /* preprocessor */
    "\033[38;5;111m", /* operator */
    "\033[48;5;24m",  /* match */
    "\033[39m"        /* normal */
};

static Theme draculaTheme =
{
    "\033[38;5;141m",
    "\033[38;5;81m",
    "\033[38;5;221m",
    "\033[38;5;215m",
    "\033[38;5;103m",
    "\033[38;5;204m",
    "\033[38;5;117m",
    "\033[48;5;60m",
    "\033[38;5;253m"
};

static Theme nordTheme =
{
    "\033[38;5;110m",
    "\033[38;5;81m",
    "\033[38;5;180m",
    "\033[38;5;179m",
    "\033[38;5;109m",
    "\033[38;5;151m",
    "\033[38;5;117m",
    "\033[48;5;24m",
    "\033[38;5;252m"
};

static Theme gruvboxTheme =
{
    "\033[38;5;167m",
    "\033[38;5;142m",
    "\033[38;5;214m",
    "\033[38;5;208m",
    "\033[38;5;245m",
    "\033[38;5;172m",
    "\033[38;5;109m",
    "\033[48;5;237m",
    "\033[38;5;223m"
};

static Theme lightTheme =
{
    "\033[34m",
    "\033[36m",
    "\033[31m",
    "\033[35m",
    "\033[90m",
    "\033[33m",
    "\033[32m",
    "\033[47m",
    "\033[30m"
};

static Theme *themes[] =
{
    &defaultTheme,
    &draculaTheme,
    &nordTheme,
    &gruvboxTheme,
    &lightTheme
};

static int currentThemeIndex = 0;

static Theme *activeTheme =
    &defaultTheme;

void themeSet(ThemeType theme)
{
    switch (theme)
    {
        case THEME_DRACULA:
    activeTheme = &draculaTheme;
    currentThemeIndex = 1;
    break;

case THEME_NORD:
    activeTheme = &nordTheme;
    currentThemeIndex = 2;
    break;

case THEME_GRUVBOX:
    activeTheme = &gruvboxTheme;
    currentThemeIndex = 3;
    break;

case THEME_LIGHT:
    activeTheme = &lightTheme;
    currentThemeIndex = 4;
    break;

default:
    activeTheme = &defaultTheme;
    currentThemeIndex = 0;
    break;
    }
}

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

const char *themeCurrentName(void)
{
    if (activeTheme == &defaultTheme)
        return "default";

    if (activeTheme == &draculaTheme)
        return "dracula";

    if (activeTheme == &nordTheme)
        return "nord";

    if (activeTheme == &gruvboxTheme)
        return "gruvbox";

    if (activeTheme == &lightTheme)
        return "light";

    return "unknown";
}

int themeNext(void)
{
    currentThemeIndex++;

    if (currentThemeIndex >= 5)
        currentThemeIndex = 0;

    activeTheme =
        themes[currentThemeIndex];

    return 1;
}