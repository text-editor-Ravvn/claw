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
    "\033[38;2;0;0;0m\033[48;2;255;255;84m",  /* match */
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

static Theme catppuccinTheme =
{
    "\033[38;5;203m",
    "\033[38;5;111m",
    "\033[38;5;150m",
    "\033[38;5;216m",
    "\033[38;5;103m",
    "\033[38;5;183m",
    "\033[38;5;189m",
    "\033[38;5;0m\033[48;5;183m",
    "\033[38;5;189m"
};

static Theme tokyoNightTheme =
{
    "\033[38;5;141m",
    "\033[38;5;110m",
    "\033[38;5;114m",
    "\033[38;5;179m",
    "\033[38;5;60m",
    "\033[38;5;111m",
    "\033[38;5;117m",
    "\033[38;5;0m\033[48;5;110m",
    "\033[38;5;189m"
};

static Theme monokaiTheme =
{
    "\033[38;5;197m",
    "\033[38;5;81m",
    "\033[38;5;186m",
    "\033[38;5;141m",
    "\033[38;5;244m",
    "\033[38;5;208m",
    "\033[38;5;81m",
    "\033[38;5;0m\033[48;5;221m",
    "\033[38;5;253m"
};

static Theme solarizedTheme =
{
    "\033[38;5;166m",
    "\033[38;5;37m",
    "\033[38;5;37m",
    "\033[38;5;136m",
    "\033[38;5;244m",
    "\033[38;5;166m",
    "\033[38;5;37m",
    "\033[38;5;0m\033[48;5;136m",
    "\033[38;5;239m"
};

static Theme kanagawaTheme =
{
    "\033[38;2;149;127;184m",
    "\033[38;2;127;180;202m",
    "\033[38;2;152;187;108m",
    "\033[38;2;230;195;132m",
    "\033[38;2;114;113;125m",
    "\033[38;2;126;156;216m",
    "\033[38;2;196;167;231m",
    "\033[38;2;29;32;46m\033[48;2;210;187;142m",
    "\033[38;2;220;215;186m"
};

static Theme *themes[] =
{
    &defaultTheme,
    &draculaTheme,
    &nordTheme,
    &gruvboxTheme,
    &lightTheme,
    &catppuccinTheme,
    &tokyoNightTheme,
    &monokaiTheme,
    &solarizedTheme,
    &kanagawaTheme
};

#define THEME_COUNT (sizeof(themes)/sizeof(themes[0]))

static int currentThemeIndex = 0;

static Theme *activeTheme =
    &defaultTheme;

void themeSet(ThemeType theme)
{
    if(theme < 0 || theme >= THEME_COUNT)
       theme = THEME_DEFAULT;

    currentThemeIndex = (int)theme;
    activeTheme = themes[currentThemeIndex];

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
    static const char *themeNames[] =
    {
        "default",
        "dracula",
        "nord",
        "gruvbox",
        "light",
        "catppuccin",
        "tokyo-night",
        "monokai",
        "solarized",
        "kanagawa"
    };

    if (currentThemeIndex < 0 ||
        currentThemeIndex >= (int)THEME_COUNT)
        return "unknown";

    return themeNames[currentThemeIndex];
}

int themeNext(void)
{
    currentThemeIndex++;

    if (currentThemeIndex >= (int)THEME_COUNT)
        currentThemeIndex = 0;

    activeTheme = themes[currentThemeIndex];

    return 1;
}