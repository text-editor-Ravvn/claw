/* Claw Remote File Editing & Transport Protocol Header (Phase 12) */
#ifndef REMOTE_H
#define REMOTE_H

#include <stddef.h>
#include <time.h>

#define REMOTE_MAX_DIR_ENTRIES 256
#define REMOTE_CACHE_DIR ".claw/cache/remote"

/* Remote URI parsed representation */
typedef struct
{
    char scheme[16];   /* "claw", "sftp", "ssh", "file" */
    char user[64];     /* optional username */
    char host[128];    /* hostname or IP */
    int port;          /* port number (e.g. 8765 for claw, 22 for sftp) */
    char path[512];    /* remote path */
    char rawUri[1024]; /* original full URI string */
} RemoteUri;

/* Remote directory entry */
typedef struct
{
    char name[256];
    int isDir;
    size_t size;
    time_t mtime;
} RemoteDirEntry;

/* Remote file metadata */
typedef struct
{
    size_t size;
    int isDir;
    int permissions;
    time_t mtime;
} RemoteFileInfo;

/* Remote connection / session state */
typedef struct
{
    int connected;
    RemoteUri currentUri;
    char sessionToken[64];
    int latencyMs;
} RemoteSession;

/* Remote Open Prompt State */
typedef struct
{
    int active;
    char input[512];
    int inputLen;
    char message[128];
} RemotePromptState;

extern RemotePromptState remotePrompt;
extern RemoteSession remoteSession;

/* --- URI & Path Parsing --- */
int remoteParseUri(const char *uriStr, RemoteUri *outUri);
int remoteBuildUri(const RemoteUri *uri, char *outStr, size_t outSize);
int remoteIsRemotePath(const char *path);

/* --- Transport Protocol & Cache Abstraction --- */
int remoteInit(void);
void remoteShutdown(void);
int remoteFetchFile(const char *remoteUriStr, char *localCachePath, size_t localCacheSize);
int remoteSaveFile(const char *remoteUriStr, const char *localFilePath);
int remoteListDirectory(const char *remoteUriStr, RemoteDirEntry *outEntries, int maxEntries, int *outCount);
int remoteStatFile(const char *remoteUriStr, RemoteFileInfo *outInfo);
int remoteGetLocalCachePath(const char *remoteUriStr, char *outPath, size_t outSize);
void remoteCacheClear(void);

/* --- Interactive Remote Open Dialog --- */
void remoteOpenPrompt(void);
void remoteClosePrompt(void);
int remotePromptIsActive(void);
void remotePromptHandleKey(int key, int modifiers);
void remotePromptDraw(void);

#endif /* REMOTE_H */
