/* Claw Project Template Generator & Scaffolding System (Phase 12) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

#include "templates.h"
#include "editor.h"
#include "viewport.h"
#include "render.h"
#include "buffer_manager.h"
#include "input.h"

TemplateWizardState templateWizard = {0};

static TemplateInfo templateRegistry[TEMPLATES_MAX];
static int templateRegistryCount = 0;

static void registerBuiltinTemplate(const char *id, const char *name, const char *desc, const char *lang, const char **files, int fileCount)
{
    if (templateRegistryCount >= TEMPLATES_MAX)
        return;

    TemplateInfo *t = &templateRegistry[templateRegistryCount++];
    snprintf(t->id, sizeof(t->id), "%s", id);
    snprintf(t->name, sizeof(t->name), "%s", name);
    snprintf(t->description, sizeof(t->description), "%s", desc);
    snprintf(t->language, sizeof(t->language), "%s", lang);
    t->fileCount = fileCount;
    for (int i = 0; i < fileCount && i < TEMPLATE_FILES_MAX; i++)
    {
        snprintf(t->defaultFiles[i], sizeof(t->defaultFiles[i]), "%s", files[i]);
    }
}

int templatesInit(void)
{
    templateRegistryCount = 0;

    const char *cFiles[] = {"Makefile", "src/main.c", "include/project.h", ".claw/project.conf", ".gitignore", "README.md"};
    registerBuiltinTemplate("c", "C Application", "Modular C project with Makefile, POSIX headers, and Claw config", "c", cFiles, 6);

    const char *pyFiles[] = {"pyproject.toml", "src/main.py", "src/__init__.py", "tests/test_main.py", "requirements.txt", ".gitignore", "README.md"};
    registerBuiltinTemplate("python", "Python Package", "Modern Python application with pyproject.toml and pytest suite", "python", pyFiles, 7);

    const char *rsFiles[] = {"Cargo.toml", "src/main.rs", ".gitignore", "README.md"};
    registerBuiltinTemplate("rust", "Rust Binary", "Rust Cargo project with fast binary configuration", "rust", rsFiles, 4);

    const char *goFiles[] = {"go.mod", "main.go", "Makefile", ".gitignore", "README.md"};
    registerBuiltinTemplate("go", "Go Module", "Go service with go.mod module definitions and build targets", "go", goFiles, 5);

    const char *webFiles[] = {"index.html", "styles.css", "app.js", "README.md"};
    registerBuiltinTemplate("web", "Web Frontend", "Modern HTML5, responsive CSS3 styles, and vanilla JS app", "html", webFiles, 4);

    return templateRegistryCount;
}

int templatesCount(void)
{
    if (templateRegistryCount == 0)
        templatesInit();
    return templateRegistryCount;
}

const TemplateInfo *templatesGet(int index)
{
    if (templateRegistryCount == 0)
        templatesInit();
    if (index >= 0 && index < templateRegistryCount)
        return &templateRegistry[index];
    return NULL;
}

const TemplateInfo *templatesFindById(const char *id)
{
    if (!id)
        return NULL;
    if (templateRegistryCount == 0)
        templatesInit();

    for (int i = 0; i < templateRegistryCount; i++)
    {
        if (strcasecmp(templateRegistry[i].id, id) == 0)
            return &templateRegistry[i];
    }
    return NULL;
}

static void createDirRecursive(const char *dir)
{
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", dir);
    size_t len = strlen(tmp);
    for (size_t i = 1; i < len; i++)
    {
        if (tmp[i] == '/' || tmp[i] == '\\')
        {
            char slash = tmp[i];
            tmp[i] = '\0';
#ifdef _WIN32
            char cmd[560];
            snprintf(cmd, sizeof(cmd), "mkdir \"%s\" 2>nul", tmp);
            system(cmd);
#else
            mkdir(tmp, 0755);
#endif
            tmp[i] = slash;
        }
    }
#ifdef _WIN32
    char cmd[560];
    snprintf(cmd, sizeof(cmd), "mkdir \"%s\" 2>nul", tmp);
    system(cmd);
#else
    mkdir(tmp, 0755);
#endif
}

static void writeFileWithSubstitutions(const char *filePath, const char *content, const char *proj, const char *auth, const char *yr)
{
    FILE *fp = fopen(filePath, "w");
    if (!fp)
        return;

    const char *p = content;
    while (*p)
    {
        if (strncmp(p, "{{PROJECT_NAME}}", 16) == 0)
        {
            fputs(proj, fp);
            p += 16;
        }
        else if (strncmp(p, "{{AUTHOR}}", 10) == 0)
        {
            fputs(auth, fp);
            p += 10;
        }
        else if (strncmp(p, "{{YEAR}}", 8) == 0)
        {
            fputs(yr, fp);
            p += 8;
        }
        else
        {
            fputc(*p++, fp);
        }
    }
    fclose(fp);
}

int templatesScaffold(const char *templateId, const char *targetDir, const char *projectName, const char *author)
{
    const TemplateInfo *t = templatesFindById(templateId);
    if (!t)
        return 0;

    const char *proj = (projectName && projectName[0]) ? projectName : "my_project";
    const char *auth = (author && author[0]) ? author : "Claw Developer";
    const char *dir = (targetDir && targetDir[0]) ? targetDir : proj;
    char yr[8] = "2026";

    createDirRecursive(dir);

    char pathBuf[512];

    if (strcmp(t->id, "c") == 0)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s/src", dir);
        createDirRecursive(pathBuf);
        snprintf(pathBuf, sizeof(pathBuf), "%s/include", dir);
        createDirRecursive(pathBuf);
        snprintf(pathBuf, sizeof(pathBuf), "%s/.claw", dir);
        createDirRecursive(pathBuf);

        snprintf(pathBuf, sizeof(pathBuf), "%s/Makefile", dir);
        writeFileWithSubstitutions(pathBuf,
            "CC = gcc\nCFLAGS = -Wall -Wextra -std=c11 -Iinclude\nTARGET = {{PROJECT_NAME}}\nSRC = src/main.c\n\nall: $(TARGET)\n\n$(TARGET): $(SRC)\n\t$(CC) $(CFLAGS) $(SRC) -o $(TARGET)\n\nclean:\n\trm -f $(TARGET)\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/include/project.h", dir);
        writeFileWithSubstitutions(pathBuf,
            "#ifndef PROJECT_H\n#define PROJECT_H\n\n#define APP_NAME \"{{PROJECT_NAME}}\"\n#define APP_VERSION \"1.0.0\"\n#define APP_AUTHOR \"{{AUTHOR}}\"\n\nvoid app_init(void);\n\n#endif\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/src/main.c", dir);
        writeFileWithSubstitutions(pathBuf,
            "#include <stdio.h>\n#include \"project.h\"\n\nvoid app_init(void) {\n    printf(\"Initializing %s v%s by %s\\n\", APP_NAME, APP_VERSION, APP_AUTHOR);\n}\n\nint main(void) {\n    app_init();\n    printf(\"Hello, {{PROJECT_NAME}}!\\n\");\n    return 0;\n}\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/.claw/project.conf", dir);
        writeFileWithSubstitutions(pathBuf,
            "[project]\nname = {{PROJECT_NAME}}\nversion = 1.0.0\nauthor = {{AUTHOR}}\nbuild = make\ntest = make test\nrun = ./{{PROJECT_NAME}}\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/.gitignore", dir);
        writeFileWithSubstitutions(pathBuf, "*.o\n{{PROJECT_NAME}}\n{{PROJECT_NAME}}.exe\n.claw/cache/\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/README.md", dir);
        writeFileWithSubstitutions(pathBuf, "# {{PROJECT_NAME}}\n\nAuthor: {{AUTHOR}} ({{YEAR}})\n\nBuilt with Claw Developer Platform.\n", proj, auth, yr);
    }
    else if (strcmp(t->id, "python") == 0)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s/src", dir);
        createDirRecursive(pathBuf);
        snprintf(pathBuf, sizeof(pathBuf), "%s/tests", dir);
        createDirRecursive(pathBuf);

        snprintf(pathBuf, sizeof(pathBuf), "%s/pyproject.toml", dir);
        writeFileWithSubstitutions(pathBuf,
            "[project]\nname = \"{{PROJECT_NAME}}\"\nversion = \"0.1.0\"\ndescription = \"{{PROJECT_NAME}} package\"\nauthors = [{ name = \"{{AUTHOR}}\" }]\nreadme = \"README.md\"\nrequires-python = \">=3.9\"\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/src/__init__.py", dir);
        writeFileWithSubstitutions(pathBuf, "\"\"\"{{PROJECT_NAME}} package\"\"\"\n__version__ = \"0.1.0\"\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/src/main.py", dir);
        writeFileWithSubstitutions(pathBuf,
            "def run() -> None:\n    print(\"Hello from {{PROJECT_NAME}} by {{AUTHOR}}!\")\n\nif __name__ == '__main__':\n    run()\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/tests/test_main.py", dir);
        writeFileWithSubstitutions(pathBuf,
            "import unittest\nfrom src.main import run\n\nclass TestApp(unittest.TestCase):\n    def test_run(self):\n        self.assertTrue(callable(run))\n\nif __name__ == '__main__':\n    unittest.main()\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/requirements.txt", dir);
        writeFileWithSubstitutions(pathBuf, "pytest>=7.0.0\nblack>=23.0.0\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/.gitignore", dir);
        writeFileWithSubstitutions(pathBuf, "__pycache__/\n*.pyc\n.pytest_cache/\n.venv/\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/README.md", dir);
        writeFileWithSubstitutions(pathBuf, "# {{PROJECT_NAME}}\n\nPython application by {{AUTHOR}}.\n", proj, auth, yr);
    }
    else if (strcmp(t->id, "rust") == 0)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s/src", dir);
        createDirRecursive(pathBuf);

        snprintf(pathBuf, sizeof(pathBuf), "%s/Cargo.toml", dir);
        writeFileWithSubstitutions(pathBuf,
            "[package]\nname = \"{{PROJECT_NAME}}\"\nversion = \"0.1.0\"\nedition = \"2021\"\nauthors = [\"{{AUTHOR}}\"]\n\n[dependencies]\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/src/main.rs", dir);
        writeFileWithSubstitutions(pathBuf,
            "fn main() {\n    println!(\"Hello from {{PROJECT_NAME}}!\");\n}\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/.gitignore", dir);
        writeFileWithSubstitutions(pathBuf, "/target\n**/*.rs.bk\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/README.md", dir);
        writeFileWithSubstitutions(pathBuf, "# {{PROJECT_NAME}}\n\nRust binary by {{AUTHOR}}.\n", proj, auth, yr);
    }
    else if (strcmp(t->id, "go") == 0)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s/go.mod", dir);
        writeFileWithSubstitutions(pathBuf, "module {{PROJECT_NAME}}\n\ngo 1.21\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/main.go", dir);
        writeFileWithSubstitutions(pathBuf,
            "package main\n\nimport \"fmt\"\n\nfunc main() {\n    fmt.Println(\"Hello, {{PROJECT_NAME}}!\")\n}\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/Makefile", dir);
        writeFileWithSubstitutions(pathBuf, "build:\n\tgo build -o bin/{{PROJECT_NAME}} main.go\n\nrun:\n\tgo run main.go\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/.gitignore", dir);
        writeFileWithSubstitutions(pathBuf, "/bin/\n", proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/README.md", dir);
        writeFileWithSubstitutions(pathBuf, "# {{PROJECT_NAME}}\n\nGo module by {{AUTHOR}}.\n", proj, auth, yr);
    }
    else if (strcmp(t->id, "web") == 0)
    {
        snprintf(pathBuf, sizeof(pathBuf), "%s/index.html", dir);
        writeFileWithSubstitutions(pathBuf,
            "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <title>{{PROJECT_NAME}}</title>\n    <link rel=\"stylesheet\" href=\"styles.css\">\n</head>\n<body>\n    <div class=\"container\">\n        <h1>Welcome to {{PROJECT_NAME}}</h1>\n        <p>Created by {{AUTHOR}}</p>\n    </div>\n    <script src=\"app.js\"></script>\n</body>\n</html>\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/styles.css", dir);
        writeFileWithSubstitutions(pathBuf,
            "body {\n    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\n    background: #1e1e2e;\n    color: #cdd6f4;\n    display: flex;\n    justify-content: center;\n    align-items: center;\n    height: 100vh;\n    margin: 0;\n}\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/app.js", dir);
        writeFileWithSubstitutions(pathBuf,
            "console.log('{{PROJECT_NAME}} initialized');\n",
            proj, auth, yr);

        snprintf(pathBuf, sizeof(pathBuf), "%s/README.md", dir);
        writeFileWithSubstitutions(pathBuf, "# {{PROJECT_NAME}}\n\nWeb frontend by {{AUTHOR}}.\n", proj, auth, yr);
    }

    return 1;
}

/* --- Interactive New Project Wizard Modal --- */

void templateWizardOpen(void)
{
    templatesInit();
    templateWizard.active = 1;
    templateWizard.selectedIndex = 0;
    templateWizard.focusedField = 0;
    snprintf(templateWizard.projectName, sizeof(templateWizard.projectName), "my_project");
    templateWizard.nameLen = (int)strlen(templateWizard.projectName);
    snprintf(templateWizard.targetDir, sizeof(templateWizard.targetDir), "./my_project");
    templateWizard.dirLen = (int)strlen(templateWizard.targetDir);
    snprintf(templateWizard.author, sizeof(templateWizard.author), "Claw Developer");
    templateWizard.authorLen = (int)strlen(templateWizard.author);
    templateWizard.statusMessage[0] = '\0';
    editorSetStatusMessage("");
}

void templateWizardClose(void)
{
    templateWizard.active = 0;
    renderForceClear();
}

int templateWizardIsActive(void)
{
    return templateWizard.active;
}

void templateWizardDraw(void)
{
    if (!templateWizard.active)
        return;

    int width = 64;
    int height = 15;
    int startCol = (viewport.screenCols - width) / 2;
    if (startCol < 1) startCol = 1;
    int startRow = (viewport.screenRows - height) / 2;
    if (startRow < 2) startRow = 2;

    const TemplateInfo *curr = templatesGet(templateWizard.selectedIndex);

    printf("\033[%d;%dH\033[1;36m+------------------ New Project Wizard ------------------------+\033[0m", startRow++, startCol);
    printf("\033[%d;%dH\033[1;36m|\033[0m  Template: \033[1;%sm< %-16.16s >\033[0m (%d/%d)                    \033[1;36m|\033[0m",
           startRow++, startCol,
           (templateWizard.focusedField == 0) ? "33;7" : "37",
           curr ? curr->name : "None",
           templateWizard.selectedIndex + 1, templateRegistryCount);

    printf("\033[%d;%dH\033[1;36m|\033[0m  \033[90m%-58.58s\033[0m  \033[1;36m|\033[0m",
           startRow++, startCol, curr ? curr->description : "");

    printf("\033[%d;%dH\033[1;36m+--------------------------------------------------------------+\033[0m", startRow++, startCol);

    /* Project Name Field */
    printf("\033[%d;%dH\033[1;36m|\033[0m  Project Name: \033[%sm%-42.42s\033[0m  \033[1;36m|\033[0m",
           startRow++, startCol,
           (templateWizard.focusedField == 1) ? "7" : "0",
           templateWizard.projectName);

    /* Target Directory Field */
    printf("\033[%d;%dH\033[1;36m|\033[0m  Target Dir:   \033[%sm%-42.42s\033[0m  \033[1;36m|\033[0m",
           startRow++, startCol,
           (templateWizard.focusedField == 2) ? "7" : "0",
           templateWizard.targetDir);

    /* Author Field */
    printf("\033[%d;%dH\033[1;36m|\033[0m  Author:       \033[%sm%-42.42s\033[0m  \033[1;36m|\033[0m",
           startRow++, startCol,
           (templateWizard.focusedField == 3) ? "7" : "0",
           templateWizard.author);

    printf("\033[%d;%dH\033[1;36m+--------------------------------------------------------------+\033[0m", startRow++, startCol);

    if (templateWizard.statusMessage[0])
    {
        printf("\033[%d;%dH\033[1;32m|\033[0m  \033[1;33m%-58.58s\033[0m  \033[1;32m|\033[0m",
               startRow++, startCol, templateWizard.statusMessage);
    }
    else
    {
        printf("\033[%d;%dH\033[1;36m|\033[0m  Files: %-51.51s \033[1;36m|\033[0m",
               startRow++, startCol,
               curr ? curr->defaultFiles[0] : "");
    }

    printf("\033[%d;%dH\033[1;36m+-- [Tab] Next Field  [Enter] Create  [Esc] Cancel ------------+\033[0m", startRow, startCol);
}

void templateWizardHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (key == 27)
    {
        templateWizardClose();
        return;
    }

    if (key == '\t')
    {
        templateWizard.focusedField = (templateWizard.focusedField + 1) % 4;
        return;
    }

    /* Template selection field */
    if (templateWizard.focusedField == 0)
    {
        if (key == ARROW_LEFT || key == 'h')
        {
            if (templateWizard.selectedIndex > 0)
                templateWizard.selectedIndex--;
            else
                templateWizard.selectedIndex = templateRegistryCount - 1;
            return;
        }
        if (key == ARROW_RIGHT || key == 'l')
        {
            if (templateWizard.selectedIndex < templateRegistryCount - 1)
                templateWizard.selectedIndex++;
            else
                templateWizard.selectedIndex = 0;
            return;
        }
    }

    /* Enter creates project */
    if (key == '\r' || key == '\n')
    {
        const TemplateInfo *t = templatesGet(templateWizard.selectedIndex);
        if (t)
        {
            if (templatesScaffold(t->id, templateWizard.targetDir, templateWizard.projectName, templateWizard.author))
            {
                templateWizardClose();
                char mainFilePath[512];
                snprintf(mainFilePath, sizeof(mainFilePath), "%s/%s", templateWizard.targetDir, t->defaultFiles[1]);
                bufferManagerOpen(mainFilePath);
                char msg[256];
                snprintf(msg, sizeof(msg), "Created and scaffolded %s project '%s' successfully!", t->name, templateWizard.projectName);
                editorSetStatusMessage(msg);
                return;
            }
        }
        return;
    }

    /* Text input handling for fields 1, 2, 3 */
    char *buf = NULL;
    int *len = NULL;
    int maxLen = 63;

    if (templateWizard.focusedField == 1)
    {
        buf = templateWizard.projectName;
        len = &templateWizard.nameLen;
        maxLen = sizeof(templateWizard.projectName) - 1;
    }
    else if (templateWizard.focusedField == 2)
    {
        buf = templateWizard.targetDir;
        len = &templateWizard.dirLen;
        maxLen = sizeof(templateWizard.targetDir) - 1;
    }
    else if (templateWizard.focusedField == 3)
    {
        buf = templateWizard.author;
        len = &templateWizard.authorLen;
        maxLen = sizeof(templateWizard.author) - 1;
    }

    if (buf && len)
    {
        if (key == 127 || key == 8)
        {
            if (*len > 0)
            {
                (*len)--;
                buf[*len] = '\0';
            }
            return;
        }
        if (key >= 32 && key <= 126 && *len < maxLen)
        {
            buf[(*len)++] = (char)key;
            buf[*len] = '\0';
            return;
        }
    }
}
