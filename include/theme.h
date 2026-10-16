#ifndef THEME_H
#define THEME_H

#include "highlight.h"

typedef struct
{
    const char *keyword;
    const char *type;
    const char *string;
    const char *number;
    const char *comment;
    const char *preprocessor;
    const char *operator;
    const char *match;
    const char *normal;
}
Theme;

typedef enum
{
    THEME_DEFAULT,
    THEME_DRACULA,
    THEME_NORD,
    THEME_GRUVBOX,
    THEME_LIGHT,
    THEME_CATPPUCCIN,
    THEME_TOKYO_NIGHT,
    THEME_MONOKAI,
    THEME_SOLARIZED,
    THEME_KANAGAWA
} ThemeType;

void themeSet(ThemeType theme);

void themeInit(void);

int themeNext(void);

const char *themeCurrentName(void);

const char *themeColor(
    int hlType
);

#endif