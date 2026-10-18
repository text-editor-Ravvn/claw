#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project.h"
#include "workspace.h"

ProjectConfig currentProject = {0};
ProjectOutput projectOutput = {0};

static char *trimWhitespace(char *text)
{
    char *end;

    while (*text == ' ' || *text == '\t')
        text++;

    end = text + strlen(text);
    while (end > text &&
           (end[-1] == ' ' || end[-1] == '\t' ||
            end[-1] == '\r' || end[-1] == '\n'))
    {
        *--end = '\0';
    }

    return text;
}

static int copyValue(char *destination, size_t destinationSize,
                     const char *value)
{
    if (destinationSize == 0 || strlen(value) >= destinationSize)
        return 0;

    snprintf(destination, destinationSize, "%s", value);
    return 1;
}

static int parseProjectFile(FILE *file, ProjectConfig *project)
{
    char line[PROJECT_COMMAND_MAX + PROJECT_TASK_NAME_MAX + 64];

    while (fgets(line, sizeof(line), file))
    {
        char *entry = trimWhitespace(line);
        char *equals;
        char *key;
        char *value;

        if (*entry == '\0' || *entry == '#')
            continue;

        equals = strchr(entry, '=');
        if (!equals)
            continue;

        *equals = '\0';
        key = trimWhitespace(entry);
        value = trimWhitespace(equals + 1);

        if (strncmp(key, "task.", 5) == 0)
        {
            const char *taskName = key + 5;
            ProjectTask *task;

            if (*taskName == '\0' || project->taskCount >= PROJECT_MAX_TASKS)
                return 0;

            task = &project->tasks[project->taskCount];
            if (!copyValue(task->name, sizeof(task->name), taskName) ||
                !copyValue(task->command, sizeof(task->command), value))
            {
                return 0;
            }

            project->taskCount++;
        }
        else if (strcmp(key, "name") == 0 &&
            !copyValue(project->name, sizeof(project->name), value))
            return 0;
        else if (strcmp(key, "type") == 0 &&
                 !copyValue(project->type, sizeof(project->type), value))
            return 0;
        else if (strcmp(key, "build") == 0 &&
                 !copyValue(project->buildCommand,
                            sizeof(project->buildCommand), value))
            return 0;
        else if (strcmp(key, "run") == 0 &&
                 !copyValue(project->runCommand,
                            sizeof(project->runCommand), value))
            return 0;
        else if (strcmp(key, "test") == 0 &&
                 !copyValue(project->testCommand,
                            sizeof(project->testCommand), value))
            return 0;
    }

    return ferror(file) ? 0 : 1;
}

static void setProjectOutput(const char *text, size_t length, int exitCode)
{
    if (length >= sizeof(projectOutput.text))
        length = sizeof(projectOutput.text) - 1;

    memcpy(projectOutput.text, text, length);
    projectOutput.text[length] = '\0';
    projectOutput.length = length;
    projectOutput.exitCode = exitCode;
}

static int runProjectCommand(const char *commandText)
{
    char command[PROJECT_COMMAND_MAX + PROJECT_PATH_MAX + 64];
    char outputPath[PROJECT_PATH_MAX + 32];
    char output[PROJECT_OUTPUT_MAX];
    FILE *outputFile;
    size_t outputLength = 0;
    int length;

    if (!currentProject.active || !commandText || commandText[0] == '\0')
        return 0;

    if (snprintf(outputPath, sizeof(outputPath),
                 "%s/.claw/project-output.log",
                 currentProject.rootPath) >= (int)sizeof(outputPath))
    {
        const char message[] = "Project output path is too long\n";
        setProjectOutput(message, strlen(message), -1);
        return 0;
    }

#ifdef _WIN32
    length = snprintf(command, sizeof(command),
                      "cd /d \"%s\" && (%s) > \"%s\" 2>&1",
                      currentProject.rootPath, commandText, outputPath);
#else
    length = snprintf(command, sizeof(command),
                      "cd \"%s\" && (%s) > \"%s\" 2>&1",
                      currentProject.rootPath, commandText, outputPath);
#endif

    if (length < 0 || (size_t)length >= sizeof(command))
    {
        const char message[] = "Project command is too long\n";
        setProjectOutput(message, strlen(message), -1);
        return 0;
    }

    projectOutput.active = 0;
    projectOutput.length = 0;
    projectOutput.text[0] = '\0';
    projectOutput.exitCode = system(command);

    outputFile = fopen(outputPath, "rb");
    if (!outputFile)
    {
        const char message[] = "Project command produced no output\n";
        setProjectOutput(message, strlen(message), projectOutput.exitCode);
        return projectOutput.exitCode == 0;
    }

    while (outputLength < sizeof(output) - 1 &&
           fgets(output + outputLength,
                 (int)(sizeof(output) - outputLength),
                 outputFile))
    {
        outputLength = strlen(output);
    }

    fclose(outputFile);
    if (outputLength == 0)
    {
        const char message[] = "Project command produced no output\n";
        setProjectOutput(message, strlen(message), projectOutput.exitCode);
        return projectOutput.exitCode == 0;
    }

    setProjectOutput(output, outputLength, projectOutput.exitCode);
    return projectOutput.exitCode == 0;
}

int projectLoad(const char *workspaceRoot)
{
    char configPath[PROJECT_PATH_MAX];
    FILE *file;
    ProjectConfig loaded = {0};

    memset(&currentProject, 0, sizeof(currentProject));

    if (!workspaceRoot || !*workspaceRoot)
        return 0;

    if (snprintf(configPath, sizeof(configPath),
                 "%s/.claw/project.conf", workspaceRoot) >=
        (int)sizeof(configPath))
    {
        return 0;
    }

    file = fopen(configPath, "r");
    if (!file)
        return 0;

    if (!parseProjectFile(file, &loaded))
    {
        fclose(file);
        return 0;
    }

    fclose(file);
    if (!copyValue(loaded.rootPath, sizeof(loaded.rootPath), workspaceRoot) ||
        !copyValue(loaded.configPath, sizeof(loaded.configPath), configPath))
    {
        return 0;
    }
    loaded.active = 1;
    currentProject = loaded;
    return 1;
}

int projectBuild(void)
{
    const char *buildCommand;

    if (!currentProject.active)
        return 0;

    buildCommand = projectGetBuildCommand();
    if (!buildCommand || buildCommand[0] == '\0')
        return 0;

    return runProjectCommand(buildCommand);
}

int projectRunTask(const char *name)
{
    const char *command = NULL;

    if (!name || !*name || !currentProject.active)
        return 0;

    if (strcmp(name, "build") == 0)
        command = currentProject.buildCommand;
    else if (strcmp(name, "run") == 0)
        command = currentProject.runCommand;
    else if (strcmp(name, "test") == 0)
        command = currentProject.testCommand;
    else
    {
        for (size_t i = 0; i < currentProject.taskCount; i++)
        {
            if (strcmp(currentProject.tasks[i].name, name) == 0)
            {
                command = currentProject.tasks[i].command;
                break;
            }
        }
    }

    if (!command || command[0] == '\0')
    {
        const char message[] = "Project task is not configured\n";
        setProjectOutput(message, strlen(message), -1);
        return 0;
    }

    return runProjectCommand(command);
}

void projectOutputOpen(void)
{
    projectOutput.active = 1;
}

void projectOutputClose(void)
{
    projectOutput.active = 0;
}

int projectOutputIsActive(void)
{
    return projectOutput.active;
}

size_t projectTaskCount(void)
{
    return currentProject.active ? currentProject.taskCount : 0;
}

const ProjectTask *projectGetTask(size_t index)
{
    if (!currentProject.active || index >= currentProject.taskCount)
        return NULL;

    return &currentProject.tasks[index];
}

void projectInit(void)
{
    memset(&currentProject, 0, sizeof(currentProject));

    if (workspaceIsActive())
        projectLoad(workspaceGetRoot());
}

const char *projectGetName(void)
{
    return currentProject.active ? currentProject.name : NULL;
}

const char *projectGetType(void)
{
    return currentProject.active ? currentProject.type : NULL;
}

const char *projectGetBuildCommand(void)
{
    return currentProject.active ? currentProject.buildCommand : NULL;
}

const char *projectGetRunCommand(void)
{
    return currentProject.active ? currentProject.runCommand : NULL;
}

const char *projectGetTestCommand(void)
{
    return currentProject.active ? currentProject.testCommand : NULL;
}

const char *projectGetRoot(void)
{
    return currentProject.active ? currentProject.rootPath : NULL;
}

const char *projectGetConfigPath(void)
{
    return currentProject.active ? currentProject.configPath : NULL;
}

int projectIsActive(void)
{
    return currentProject.active;
}
