/* Claw Project Template Generator & Scaffolding Header (Phase 12) */
#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <stddef.h>

#define TEMPLATES_MAX 10
#define TEMPLATE_FILES_MAX 16

/* Template descriptor */
typedef struct
{
    char id[32];          /* "c", "python", "rust", "go", "web" */
    char name[64];        /* e.g. "C Application" */
    char description[160];/* e.g. "C project with Makefile, .claw/project.conf, modular src/include" */
    char language[32];    /* "c", "python", "rust", "go", "html" */
    int fileCount;
    char defaultFiles[TEMPLATE_FILES_MAX][64];
} TemplateInfo;

/* Interactive Project Wizard Modal State */
typedef struct
{
    int active;
    int selectedIndex;
    int focusedField;     /* 0=Template Selector, 1=Project Name, 2=Target Directory, 3=Author */
    char projectName[64];
    int nameLen;
    char targetDir[256];
    int dirLen;
    char author[64];
    int authorLen;
    char statusMessage[128];
} TemplateWizardState;

extern TemplateWizardState templateWizard;

/* --- Template Registry & Querying --- */
int templatesInit(void);
int templatesCount(void);
const TemplateInfo *templatesGet(int index);
const TemplateInfo *templatesFindById(const char *id);

/* --- Scaffolding Engine --- */
int templatesScaffold(const char *templateId, const char *targetDir, const char *projectName, const char *author);

/* --- Interactive New Project Wizard Modal --- */
void templateWizardOpen(void);
void templateWizardClose(void);
int templateWizardIsActive(void);
void templateWizardHandleKey(int key, int modifiers);
void templateWizardDraw(void);

#endif /* TEMPLATES_H */
