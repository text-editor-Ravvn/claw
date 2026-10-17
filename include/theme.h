#ifndef THEME_H
#define THEME_H

#include "highlight.h"
#include <stddef.h>

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

int themePreviewCustom(const char *name, const int codes[9]);

void themeCancelPreview(void);

int themeSaveCustom(const char *name, const int codes[9]);

int themeNext(void);

const char *themeCurrentName(void);

const char *themeColor(
    int hlType
);

size_t themeCustomCount(void);

const char *themeCustomName(size_t index);

int themeActivateCustom(size_t index);

int themeDeleteCustom(size_t index);

#endif