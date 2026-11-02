/* Claw Plugin & Extension Marketplace Header (Phase 13) */
#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include <stddef.h>

#define MARKETPLACE_MAX_PACKAGES 64
#define MARKETPLACE_CATEGORIES_COUNT 6

/* Marketplace Package representation */
typedef struct
{
    char id[64];              /* e.g. "claw-nord-theme", "claw-rust-analyzer", "claw-git-lens" */
    char name[64];            /* Display name */
    char version[32];         /* Latest available version */
    char author[64];          /* Extension author */
    char description[256];    /* Summary description */
    char category[32];        /* "Themes", "Languages", "Tooling", "LSP", "Productivity", "All" */
    int downloads;            /* Total downloads */
    int rating;               /* 1-5 star rating */
    char installedVersion[32];/* Currently installed version or empty */
    int isInstalled;          /* 1 if currently installed in plugins/ */
    int hasUpdate;            /* 1 if newer version available */
    char homepage[128];       /* Extension repository or URL */
} MarketplacePackage;

/* Interactive Marketplace UI Modal State */
typedef struct
{
    int active;
    int currentTab;           /* 0=Browse All, 1=Installed, 2=Updates */
    int selectedCategoryIndex;/* 0="All", 1="Themes", 2="Languages", 3="Tooling", 4="LSP", 5="Productivity" */
    int selectedIndex;
    int scrollOffset;
    char searchQuery[64];
    int queryLen;
    int searching;
    char statusMessage[128];
} MarketplaceState;

extern MarketplaceState marketplaceUI;

/* --- Marketplace Client Engine --- */
int marketplaceInit(void);
void marketplaceShutdown(void);
int marketplacePackageCount(void);
const MarketplacePackage *marketplaceGetPackage(int index);
const MarketplacePackage *marketplaceFindById(const char *packageId);
int marketplaceSearch(const char *query, const char *category, MarketplacePackage *outResults, int maxResults, int *outCount);

/* --- Package Operations --- */
int marketplaceInstall(const char *packageId);
int marketplaceUninstall(const char *packageId);
int marketplaceUpdate(const char *packageId);
int marketplaceCheckUpdates(int *outUpdatesCount);

/* --- Interactive Marketplace UI Modal --- */
void marketplaceOpen(void);
void marketplaceClose(void);
int marketplaceIsActive(void);
void marketplaceHandleKey(int key, int modifiers);
void marketplaceDraw(void);

#endif /* MARKETPLACE_H */
