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

void themeInit(void);

const char *themeColor(
    int hlType
);

#endif