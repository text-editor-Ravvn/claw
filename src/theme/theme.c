#include "theme.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>

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
#define MAX_CUSTOM_THEMES 32
#define THEME_NAME_SIZE 64
#define THEME_COLOR_SIZE 32

typedef struct
{
    char name[THEME_NAME_SIZE];
    char colors[9][THEME_COLOR_SIZE];
    char path[PATH_MAX];
    Theme theme;
} CustomTheme;

static CustomTheme customThemes[MAX_CUSTOM_THEMES];
static size_t customThemeCount = 0;

static int currentThemeIndex = 0;

static Theme *activeTheme =
    &defaultTheme;

static CustomTheme previewTheme;
static Theme *themeBeforePreview = NULL;
static int indexBeforePreview = 0;
static int previewActive = 0;

void themeSet(ThemeType theme)
{
    if(theme < 0 || theme >= THEME_COUNT)
       theme = THEME_DEFAULT;

    currentThemeIndex = (int)theme;
    activeTheme = themes[currentThemeIndex];

}


static char *trimThemeWhitespace(char *text)
{
    while (isspace((unsigned char)*text))
        text++;

    char *end = text + strlen(text);

    while (end > text && isspace((unsigned char)end[-1]))
        *--end = '\0';

    return text;
}

static int parseAnsiColor(const char *value, char *output, size_t outputSize)
{
    char *end = NULL;
    long code;

    if (!value || !*value || !output || outputSize < 8)
        return 0;

    errno = 0;
    code = strtol(value, &end, 10);

    if (errno != 0 || end == value || *end != '\0')
        return 0;

    if (code < 0 || code > 107)
        return 0;

    int written = snprintf(output, outputSize, "\033[%ldm", code);

    return written > 0 && (size_t)written < outputSize;
}

static int ensureThemeDirectory(const char *directory)
{
    if (!directory || !*directory)
        return 0;

    char path[PATH_MAX];

    int written = snprintf(path, sizeof(path), "%s", directory);
    if (written < 0 || (size_t)written >= sizeof(path))
        return 0;

    for (char *p = path + 1; ; p++)
    {
        if (*p == '/' || *p == '\0')
        {
            char saved = *p;
            *p = '\0';

            if (*path && mkdir(path, 0700) != 0 && errno != EEXIST)
                return 0;

            *p = saved;

            if (saved == '\0')
                break;
        }
    }

    struct stat info;
    return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

int themePreviewCustom(const char *name, const int codes[9])
{
    if (!name || !*name || !codes ||
        strlen(name) >= sizeof(previewTheme.name))
        return 0;

    CustomTheme candidate = {0};

    snprintf(candidate.name, sizeof(candidate.name), "%s", name);

    for (size_t i = 0; i < 9; i++)
    {
        char numericCode[16];

        if (codes[i] < 0 || codes[i] > 107)
            return 0;

        snprintf(numericCode, sizeof(numericCode), "%d", codes[i]);

        if (!parseAnsiColor(numericCode,
                            candidate.colors[i],
                            sizeof(candidate.colors[i])))
            return 0;
    }

    candidate.theme.keyword = candidate.colors[0];
    candidate.theme.type = candidate.colors[1];
    candidate.theme.string = candidate.colors[2];
    candidate.theme.number = candidate.colors[3];
    candidate.theme.comment = candidate.colors[4];
    candidate.theme.preprocessor = candidate.colors[5];
    candidate.theme.operator = candidate.colors[6];
    candidate.theme.match = candidate.colors[7];
    candidate.theme.normal = candidate.colors[8];

    if (!previewActive)
    {
        themeBeforePreview = activeTheme;
        indexBeforePreview = currentThemeIndex;
    }

    previewTheme = candidate;

    /*
     * Rebuild pointers after copying the candidate because Theme fields
     * point into the CustomTheme's own color buffers.
     */
    previewTheme.theme.keyword = previewTheme.colors[0];
    previewTheme.theme.type = previewTheme.colors[1];
    previewTheme.theme.string = previewTheme.colors[2];
    previewTheme.theme.number = previewTheme.colors[3];
    previewTheme.theme.comment = previewTheme.colors[4];
    previewTheme.theme.preprocessor = previewTheme.colors[5];
    previewTheme.theme.operator = previewTheme.colors[6];
    previewTheme.theme.match = previewTheme.colors[7];
    previewTheme.theme.normal = previewTheme.colors[8];

    activeTheme = &previewTheme.theme;
    previewActive = 1;

    return 1;
}

void themeCancelPreview(void)
{
    if (!previewActive)
        return;

    activeTheme = themeBeforePreview;
    currentThemeIndex = indexBeforePreview;

    themeBeforePreview = NULL;
    previewActive = 0;
}

int themeSaveCustom(const char *name, const int codes[9])
{
    if (!name || !*name || !codes ||
        strlen(name) >= THEME_NAME_SIZE ||
        customThemeCount >= MAX_CUSTOM_THEMES)
        return 0;

    /* Restrict names to safe characters for filenames. */
    for (const char *p = name; *p; p++)
    {
        if (!((*p >= 'a' && *p <= 'z') ||
              (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == ' ' || *p == '-' || *p == '_'))
            return 0;
    }

    /* Reject duplicate names, including built-in themes. */
    if (strcmp(name, "default") == 0 ||
        strcmp(name, "dracula") == 0 ||
        strcmp(name, "nord") == 0 ||
        strcmp(name, "gruvbox") == 0 ||
        strcmp(name, "light") == 0 ||
        strcmp(name, "catppuccin") == 0 ||
        strcmp(name, "tokyo-night") == 0 ||
        strcmp(name, "monokai") == 0 ||
        strcmp(name, "solarized") == 0 ||
        strcmp(name, "kanagawa") == 0)
        return 0;

    for (size_t i = 0; i < customThemeCount; i++)
    {
        if (strcmp(customThemes[i].name, name) == 0)
            return 0;
    }

    char directory[PATH_MAX];
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");

    int written;

    if (xdg && *xdg)
        written = snprintf(directory, sizeof(directory),
                           "%s/claw/themes", xdg);
    else if (home && *home)
        written = snprintf(directory, sizeof(directory),
                           "%s/.config/claw/themes", home);
    else
        return 0;

    if (written < 0 || (size_t)written >= sizeof(directory) ||
        !ensureThemeDirectory(directory))
        return 0;

    char filename[THEME_NAME_SIZE * 2];

    size_t out = 0;
    for (const char *p = name; *p && out < sizeof(filename) - 6; p++)
    {
        char c = *p;
        if (c == ' ')
            c = '-';

        if (c >= 'A' && c <= 'Z')
            c = (char)(c - 'A' + 'a');

        filename[out++] = c;
    }
    filename[out] = '\0';

    char path[PATH_MAX];
    char tempPath[PATH_MAX];

    written = snprintf(path, sizeof(path), "%s/%s.conf",
                       directory, filename);

    if (written < 0 || (size_t)written >= sizeof(path))
        return 0;

    written = snprintf(tempPath, sizeof(tempPath), "%s.tmp",
                       path);

    if (written < 0 || (size_t)written >= sizeof(tempPath))
        return 0;

    /* Never overwrite an existing file. */
    FILE *existing = fopen(path, "r");
    if (existing)
    {
        fclose(existing);
        return 0;
    }

    static const char *keys[9] = {
        "keyword", "type", "string", "number", "comment",
        "preprocessor", "operator", "match", "normal"
    };

    char parsedColors[9][THEME_COLOR_SIZE];

    for (size_t i = 0; i < 9; i++)
    {
        char numericCode[16];

        if (codes[i] < 0 || codes[i] > 107)
            return 0;

        snprintf(numericCode, sizeof(numericCode), "%d", codes[i]);

        if (!parseAnsiColor(numericCode, parsedColors[i],
                            sizeof(parsedColors[i])))
            return 0;
    }

    FILE *file = fopen(tempPath, "wx");
    if (!file)
        return 0;

    int ok = fprintf(file, "name=%s\n", name) >= 0;

    for (size_t i = 0; ok && i < 9; i++)
        ok = fprintf(file, "%s=%d\n", keys[i], codes[i]) >= 0;

    if (fflush(file) != 0)
        ok = 0;

    if (fclose(file) != 0)
        ok = 0;

    if (!ok)
    {
        remove(tempPath);
        return 0;
    }

    if (rename(tempPath, path) != 0)
    {
        remove(tempPath);
        return 0;
    }

    /* Register only after persistence succeeds. */
    CustomTheme *saved = &customThemes[customThemeCount];
    memset(saved, 0, sizeof(*saved));
    snprintf(saved->name, sizeof(saved->name), "%s", name);
    snprintf(saved->path, sizeof(saved->path), "%s", path);

    for (size_t i = 0; i < 9; i++)
        snprintf(saved->colors[i], sizeof(saved->colors[i]),
                 "%s", parsedColors[i]);

    saved->theme.keyword = saved->colors[0];
    saved->theme.type = saved->colors[1];
    saved->theme.string = saved->colors[2];
    saved->theme.number = saved->colors[3];
    saved->theme.comment = saved->colors[4];
    saved->theme.preprocessor = saved->colors[5];
    saved->theme.operator = saved->colors[6];
    saved->theme.match = saved->colors[7];
    saved->theme.normal = saved->colors[8];

    customThemeCount++;
    currentThemeIndex = (int)(THEME_COUNT + customThemeCount - 1);
    activeTheme = &saved->theme;

    themeBeforePreview = NULL;
    previewActive = 0;

    return 1;
}

int themeUpdateCustom(size_t index, const char *name, const int codes[9])
{
    if (index >= customThemeCount || !name || !*name || !codes ||
        strlen(name) >= THEME_NAME_SIZE)
        return 0;

    /* Validate the name and prevent duplicate theme names. */
    for (const char *p = name; *p; p++)
    {
        if (!((*p >= 'a' && *p <= 'z') ||
              (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == ' ' || *p == '-' || *p == '_'))
            return 0;
    }

    /* Reject names reserved for built-in themes. */
    if (strcmp(name, "default") == 0 ||
        strcmp(name, "dracula") == 0 ||
        strcmp(name, "nord") == 0 ||
        strcmp(name, "gruvbox") == 0 ||
        strcmp(name, "light") == 0 ||
        strcmp(name, "catppuccin") == 0 ||
        strcmp(name, "tokyo-night") == 0 ||
        strcmp(name, "monokai") == 0 ||
        strcmp(name, "solarized") == 0 ||
        strcmp(name, "kanagawa") == 0)
        return 0;

    static const char *keys[9] = {
        "keyword", "type", "string", "number", "comment",
        "preprocessor", "operator", "match", "normal"
    };

    for (size_t i = 0; i < customThemeCount; i++)
    {
        if (i != index && strcmp(customThemes[i].name, name) == 0)
            return 0;
    }

    char parsedColors[9][THEME_COLOR_SIZE];

    for (size_t i = 0; i < 9; i++)
    {
        char numericCode[16];

        if (codes[i] < 0 || codes[i] > 107)
            return 0;

        snprintf(numericCode, sizeof(numericCode), "%d", codes[i]);

        if (!parseAnsiColor(numericCode, parsedColors[i],
                            sizeof(parsedColors[i])))
            return 0;
    }

    CustomTheme *target = &customThemes[index];

    if (!target->path[0])
        return 0;

    /* Create a unique temporary file beside the existing theme file. */
    char tempPath[PATH_MAX];
    int written = snprintf(tempPath, sizeof(tempPath),
                           "%s.tmpXXXXXX", target->path);

    if (written < 0 || (size_t)written >= sizeof(tempPath))
        return 0;

    int fd = mkstemp(tempPath);
    if (fd < 0)
        return 0;

    FILE *file = fdopen(fd, "w");

    if (!file)
    {
        close(fd);
        remove(tempPath);
        return 0;
    }

    int ok = fprintf(file, "name=%s\n", name) >= 0;

    for (size_t i = 0; ok && i < 9; i++)
        ok = fprintf(file, "%s=%d\n", keys[i], codes[i]) >= 0;

    if (fflush(file) != 0)
        ok = 0;

    if (fclose(file) != 0)
        ok = 0;

    if (!ok || rename(tempPath, target->path) != 0)
    {
        remove(tempPath);
        return 0;
    }

    /* The file is saved. Now update the in-memory theme. */
    snprintf(target->name, sizeof(target->name), "%s", name);

    for (size_t i = 0; i < 9; i++)
        snprintf(target->colors[i], sizeof(target->colors[i]),
                 "%s", parsedColors[i]);

    target->theme.keyword = target->colors[0];
    target->theme.type = target->colors[1];
    target->theme.string = target->colors[2];
    target->theme.number = target->colors[3];
    target->theme.comment = target->colors[4];
    target->theme.preprocessor = target->colors[5];
    target->theme.operator = target->colors[6];
    target->theme.match = target->colors[7];
    target->theme.normal = target->colors[8];

    /* Finish the preview and activate the updated theme. */
    themeBeforePreview = NULL;
    previewActive = 0;
    activeTheme = &target->theme;
    currentThemeIndex = (int)(THEME_COUNT + index);

    return 1;
}

static int loadCustomThemeFile(const char *path)
{
    if (!path || customThemeCount >= MAX_CUSTOM_THEMES)
        return 0;

    FILE *file = fopen(path, "r");
    if (!file)
        return 0;

    CustomTheme candidate = {0};

    static const char *defaults[9] = {
        "\033[38;5;176m",
        "\033[38;5;81m",
        "\033[38;5;214m",
        "\033[38;5;220m",
        "\033[38;5;244m",
        "\033[38;5;208m",
        "\033[38;5;111m",
        "\033[48;5;220m",
        "\033[39m"
    };

    for (size_t i = 0; i < 9; i++)
        snprintf(candidate.colors[i], sizeof(candidate.colors[i]),
                 "%s", defaults[i]);

    static const char *keys[9] = {
        "keyword", "type", "string", "number", "comment",
        "preprocessor", "operator", "match", "normal"
    };

    int seen[9] = {0};
    int valid = 1;
    int hasName = 0;
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        if (!strchr(line, '\n') && !feof(file))
        {
            valid = 0;
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';
        char *entry = trimThemeWhitespace(line);

        if (*entry == '\0' || *entry == '#')
            continue;

        char *equals = strchr(entry, '=');
        if (!equals)
        {
            valid = 0;
            break;
        }

        *equals = '\0';
        char *key = trimThemeWhitespace(entry);
        char *value = trimThemeWhitespace(equals + 1);

        if (strcmp(key, "name") == 0)
        {
            if (hasName || !*value ||
                strlen(value) >= sizeof(candidate.name))
            {
                valid = 0;
                break;
            }

            snprintf(candidate.name, sizeof(candidate.name), "%s", value);
            hasName = 1;
            continue;
        }

        int found = 0;

        for (size_t i = 0; i < 9; i++)
        {
            if (strcmp(key, keys[i]) != 0)
                continue;

            found = 1;

            if (seen[i])
            {
                valid = 0;
                break;
            }

            char parsed[THEME_COLOR_SIZE];

            if (!parseAnsiColor(value, parsed, sizeof(parsed)))
            {
                valid = 0;
                break;
            }

            snprintf(candidate.colors[i], sizeof(candidate.colors[i]),
                     "%s", parsed);
            seen[i] = 1;
            break;
        }

        if (!found || !valid)
        {
            valid = 0;
            break;
        }
    }

    if (ferror(file))
        valid = 0;

    fclose(file);

    if (!valid || !hasName)
        return 0;

    customThemes[customThemeCount] = candidate;

    CustomTheme *registered = &customThemes[customThemeCount];

    snprintf(registered->path, sizeof(registered->path), "%s", path);

    registered->theme.keyword = registered->colors[0];
    registered->theme.type = registered->colors[1];
    registered->theme.string = registered->colors[2];
    registered->theme.number = registered->colors[3];
    registered->theme.comment = registered->colors[4];
    registered->theme.preprocessor = registered->colors[5];
    registered->theme.operator = registered->colors[6];
    registered->theme.match = registered->colors[7];
    registered->theme.normal = registered->colors[8];

    customThemeCount++;
    return 1;
}

static void loadCustomThemeDirectory(const char *directory)
{
    if (!directory || !*directory ||
        customThemeCount >= MAX_CUSTOM_THEMES)
        return;

    DIR *dir = opendir(directory);
    if (!dir)
        return;

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL &&
           customThemeCount < MAX_CUSTOM_THEMES)
    {
        size_t length = strlen(entry->d_name);

        if (length <= 5 ||
            strcmp(entry->d_name + length - 5, ".conf") != 0)
            continue;

        char path[PATH_MAX];

        int written = snprintf(path, sizeof(path), "%s/%s",
                              directory, entry->d_name);

        if (written < 0 || (size_t)written >= sizeof(path))
            continue;

        loadCustomThemeFile(path);
    }

    closedir(dir);
}

void themeInit(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");

    if (xdg && *xdg)
    {
        char directory[PATH_MAX];

        int written = snprintf(directory, sizeof(directory),
                               "%s/claw/themes", xdg);

        if (written >= 0 && (size_t)written < sizeof(directory))
            loadCustomThemeDirectory(directory);
    }
    else if (home && *home)
    {
        char directory[PATH_MAX];

        int written = snprintf(directory, sizeof(directory),
                               "%s/.config/claw/themes", home);

        if (written >= 0 && (size_t)written < sizeof(directory))
            loadCustomThemeDirectory(directory);
    }

    loadCustomThemeDirectory("themes");
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
    if(previewActive)
       return previewTheme.name;

    static const char *themeNames[] = {
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
        currentThemeIndex >= (int)(THEME_COUNT + customThemeCount))
        return "unknown";

    if (currentThemeIndex < (int)THEME_COUNT)
        return themeNames[currentThemeIndex];

    return customThemes[currentThemeIndex - (int)THEME_COUNT].name;
}

int themeNext(void)
{
    int totalThemes = (int)THEME_COUNT + (int)customThemeCount;

    if (totalThemes <= 0)
        return 0;

    currentThemeIndex = (currentThemeIndex + 1) % totalThemes;

    if (currentThemeIndex < (int)THEME_COUNT)
        activeTheme = themes[currentThemeIndex];
    else
        activeTheme = &customThemes[
            currentThemeIndex - (int)THEME_COUNT
        ].theme;

    return 1;
}


size_t themeCustomCount(void)
{
    return customThemeCount;
}

const char *themeCustomName(size_t index)
{
    if (index >= customThemeCount)
        return NULL;

    return customThemes[index].name;
}


int themeGetCustomColors(size_t index, int codes[9])
{
    if (index >= customThemeCount || !codes)
        return 0;

    for (size_t i = 0; i < 9; i++)
    {
        const char *color = customThemes[index].colors[i];
        int code = -1;

        if (!color || color[0] != '\033' || color[1] != '[')
            return 0;

        /* Read standard ANSI colour sequences such as ESC[31m. */
        if (sscanf(color, "\033[%dm", &code) == 1 &&
            code >= 0 && code <= 107)
        {
            codes[i] = code;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

int themeActivateCustom(size_t index)
{
    if (index >= customThemeCount)
        return 0;

    /* Cancel any preview before changing the active theme. */
    themeCancelPreview();

    currentThemeIndex = (int)(THEME_COUNT + index);
    activeTheme = &customThemes[index].theme;

    return 1;
}

int themeDeleteCustom(size_t index)
{
    if (index >= customThemeCount)
        return 0;

    /*
     * A preview is stored separately from the registry.
     * Restore the original theme before modifying the registry.
     */
    themeCancelPreview();

    CustomTheme *target = &customThemes[index];

    /* Refuse to delete themes without a recorded source file. */
    if (!target->path[0])
        return 0;

    /*
     * Delete the persistent file first. If that fails, leave
     * the in-memory registry untouched.
     */
    if (remove(target->path) != 0)
        return 0;

    /*
     * If this theme is active, switch to Default before moving
     * registry entries so activeTheme never points to stale data.
     */
    if (activeTheme == &target->theme)
    {
        activeTheme = &defaultTheme;
        currentThemeIndex = THEME_DEFAULT;
    }
    else if (currentThemeIndex > (int)(THEME_COUNT + index))
    {
        currentThemeIndex--;
    }

    /* Remove the entry while preserving the remaining themes. */
    for (size_t i = index; i + 1 < customThemeCount; i++)
        customThemes[i] = customThemes[i + 1];

    customThemeCount--;

    /*
     * Rebuild internal pointers after moving the structures.
     * Theme fields point into each CustomTheme's own color buffers.
     */
    for (size_t i = 0; i < customThemeCount; i++)
    {
        customThemes[i].theme.keyword = customThemes[i].colors[0];
        customThemes[i].theme.type = customThemes[i].colors[1];
        customThemes[i].theme.string = customThemes[i].colors[2];
        customThemes[i].theme.number = customThemes[i].colors[3];
        customThemes[i].theme.comment = customThemes[i].colors[4];
        customThemes[i].theme.preprocessor = customThemes[i].colors[5];
        customThemes[i].theme.operator = customThemes[i].colors[6];
        customThemes[i].theme.match = customThemes[i].colors[7];
        customThemes[i].theme.normal = customThemes[i].colors[8];
    }

    /* Re-point the active theme after registry compaction. */
    if (currentThemeIndex >= (int)THEME_COUNT)
    {
        size_t activeIndex =
            (size_t)(currentThemeIndex - (int)THEME_COUNT);

        if (activeIndex < customThemeCount)
            activeTheme = &customThemes[activeIndex].theme;
        else
        {
            activeTheme = &defaultTheme;
            currentThemeIndex = THEME_DEFAULT;
        }
    }

    return 1;
}

int themeExportCustom(size_t index, const char *destinationPath)
{
    if (index >= customThemeCount ||
        !destinationPath || !*destinationPath)
        return 0;

    const char *sourcePath = customThemes[index].path;

    if (!sourcePath[0] || strcmp(sourcePath, destinationPath) == 0)
        return 0;

    FILE *source = fopen(sourcePath, "rb");
    if (!source)
        return 0;

    /* Create a new destination; never overwrite an existing file. */
    FILE *destination = fopen(destinationPath, "wbx");
    if (!destination)
    {
        fclose(source);
        return 0;
    }

    char buffer[4096];
    size_t bytesRead;
    int success = 1;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, destination) != bytesRead)
        {
            success = 0;
            break;
        }
    }

    if (ferror(source))
        success = 0;

    if (fflush(destination) != 0)
        success = 0;

    if (fclose(source) != 0)
        success = 0;

    if (fclose(destination) != 0)
        success = 0;

    if (!success)
        remove(destinationPath);

    return success;
}

int themeImportCustom(const char *sourcePath)
{
    if (!sourcePath || !*sourcePath)
        return 0;

    themeInit();

    if (customThemeCount >= MAX_CUSTOM_THEMES)
        return 0;

    char directory[PATH_MAX];
    const char *xdg = getenv("XDG_CONFIG_HOME");
    const char *home = getenv("HOME");
    int written;

    if (xdg && *xdg)
        written = snprintf(directory, sizeof(directory),
                           "%s/claw/themes", xdg);
    else if (home && *home)
        written = snprintf(directory, sizeof(directory),
                           "%s/.config/claw/themes", home);
    else
        return 0;

    if (written < 0 || (size_t)written >= sizeof(directory) ||
        !ensureThemeDirectory(directory))
        return 0;

    char tempPath[PATH_MAX];
    written = snprintf(tempPath, sizeof(tempPath),
        "%s/.claw-import-XXXXXX", directory);

    if (written < 0 || (size_t)written >= sizeof(tempPath))
        return 0;

    /* Copy into a temporary managed file without overwriting anything. */
    FILE *source = fopen(sourcePath, "rb");
    if (!source)
        return 0;

    int tempFd = mkstemp(tempPath);
    if (tempFd < 0)
    {
        fclose(source);
        return 0;
    }

    FILE *destination = fdopen(tempFd, "wb");
    if (!destination)
    {
        close(tempFd);
        fclose(source);
        remove(tempPath);
        return 0;
    }

    char buffer[4096];
    size_t bytesRead;
    int ok = 1;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0)
    {
        if (fwrite(buffer, 1, bytesRead, destination) != bytesRead)
        {
            ok = 0;
            break;
        }
    }

    if (ferror(source))
        ok = 0;

    if (fflush(destination) != 0)
        ok = 0;

    if (fclose(source) != 0)
        ok = 0;

    if (fclose(destination) != 0)
        ok = 0;

    if (!ok)
    {
        remove(tempPath);
        return 0;
    }

    /* Reuse the existing parser and its color validation. */
    if (!loadCustomThemeFile(tempPath))
    {
        remove(tempPath);
        return 0;
    }

    size_t index = customThemeCount - 1;
    CustomTheme *imported = &customThemes[index];

    /* Enforce the same safe filename characters as Theme Maker. */
    for (const char *p = imported->name; *p; p++)
    {
        if (!((*p >= 'a' && *p <= 'z') ||
              (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == ' ' || *p == '-' || *p == '_'))
        {
            customThemeCount--;
            remove(tempPath);
            return 0;
        }
    }

    /* Reject duplicate custom names and built-in theme names. */
    static const char *builtInNames[] = {
        "default", "dracula", "nord", "gruvbox", "light",
        "catppuccin", "tokyo-night", "monokai", "solarized",
        "kanagawa"
    };

    for (size_t i = 0;
         i < sizeof(builtInNames) / sizeof(builtInNames[0]); i++)
    {
        if (strcmp(imported->name, builtInNames[i]) == 0)
        {
            customThemeCount--;
            remove(tempPath);
            return 0;
        }
    }

    for (size_t i = 0; i < index; i++)
    {
        if (strcmp(customThemes[i].name, imported->name) == 0)
        {
            customThemeCount--;
            remove(tempPath);
            return 0;
        }
    }

    /* Build a filename from the validated theme name. */
    char filename[THEME_NAME_SIZE * 2];
    size_t out = 0;

    for (const char *p = imported->name;
         *p && out < sizeof(filename) - 6; p++)
    {
        char c = *p;

        if (c == ' ')
            c = '-';

        if (c >= 'A' && c <= 'Z')
            c = (char)(c - 'A' + 'a');

        filename[out++] = c;
    }

    filename[out] = '\0';

    char finalPath[PATH_MAX];
    written = snprintf(finalPath, sizeof(finalPath),
                       "%s/%s.conf", directory, filename);

    if (written < 0 || (size_t)written >= sizeof(finalPath))
    {
        customThemeCount--;
        remove(tempPath);
        return 0;
    }

    /*
     * Link atomically so an existing destination is never overwritten.
     * Both paths are in the managed theme directory.
     */

    if (link(tempPath, finalPath) != 0)
    {
        customThemeCount--;
        remove(tempPath);
        return 0;
    }

    if (remove(tempPath) != 0)
    {
        /* The imported theme is valid; the temporary copy may remain. */
    }

    /* Track the managed copy, not the external source. */
    snprintf(imported->path, sizeof(imported->path), "%s", finalPath);

    themeCancelPreview();
    currentThemeIndex = (int)(THEME_COUNT + index);
    activeTheme = &imported->theme;

    return 1;
}
