/* Claw Plugin & Extension Marketplace Engine (Phase 13) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "marketplace.h"
#include "editor.h"

MarketplaceState marketplaceUI = {0};

static MarketplacePackage catalogue[MARKETPLACE_MAX_PACKAGES];
static int catalogueCount = 0;

static void registerPackage(const char *id, const char *name, const char *ver,
                            const char *author, const char *desc, const char *cat,
                            int downloads, int rating, const char *homepage)
{
    if (catalogueCount >= MARKETPLACE_MAX_PACKAGES)
        return;

    MarketplacePackage *p = &catalogue[catalogueCount++];
    memset(p, 0, sizeof(*p));
    snprintf(p->id, sizeof(p->id), "%s", id);
    snprintf(p->name, sizeof(p->name), "%s", name);
    snprintf(p->version, sizeof(p->version), "%s", ver);
    snprintf(p->author, sizeof(p->author), "%s", author);
    snprintf(p->description, sizeof(p->description), "%s", desc);
    snprintf(p->category, sizeof(p->category), "%s", cat);
    p->downloads = downloads;
    p->rating = rating;
    p->isInstalled = 0;
    p->hasUpdate = 0;
    p->installedVersion[0] = '\0';
    if (homepage)
        snprintf(p->homepage, sizeof(p->homepage), "%s", homepage);
}

int marketplaceInit(void)
{
    catalogueCount = 0;

    registerPackage("claw-nord-theme", "Nord Theme", "1.2.0", "Arctic Ice",
                    "Arctic, north-bluish clean color palette for Claw", "Themes", 1420, 5,
                    "https://github.com/claw-ecosystem/nord");

    registerPackage("claw-dracula-theme", "Dracula Official", "2.0.1", "Dracula Theme",
                    "A dark theme for Claw with vibrant neon colors", "Themes", 2850, 5,
                    "https://github.com/claw-ecosystem/dracula");

    registerPackage("claw-clangd-lsp", "Clangd C/C++ LSP", "1.5.0", "LLVM Community",
                    "C/C++ language server with intelligent completion & navigation", "LSP", 3940, 5,
                    "https://github.com/claw-ecosystem/clangd");

    registerPackage("claw-pyright-lsp", "Pyright Python LSP", "1.1.3", "Microsoft",
                    "Fast static type checker and LSP for Python", "LSP", 3120, 5,
                    "https://github.com/claw-ecosystem/pyright");

    registerPackage("claw-rust-analyzer", "Rust Analyzer", "1.8.0", "Rust Team",
                    "Full Rust language support with macro expansions and inlay hints", "LSP", 4100, 5,
                    "https://github.com/claw-ecosystem/rust-analyzer");

    registerPackage("claw-git-lens", "GitLens Pro", "1.4.0", "Claw Tools",
                    "Advanced inline git blame annotations, commit history, and stash explorer", "Tooling", 1980, 4,
                    "https://github.com/claw-ecosystem/gitlens");

    registerPackage("claw-format-tool", "Universal Formatter", "1.0.4", "DevTools",
                    "One-key formatting for C, Python, Rust, Go, JavaScript, and HTML", "Tooling", 1670, 4,
                    "https://github.com/claw-ecosystem/formatter");

    registerPackage("claw-snippets-pack", "Super Snippets Pack", "2.1.0", "Productivity Org",
                    "Over 500 hand-crafted snippets for common algorithms and patterns", "Productivity", 2200, 5,
                    "https://github.com/claw-ecosystem/snippets");

    return catalogueCount;
}

void marketplaceShutdown(void)
{
    catalogueCount = 0;
}

int marketplacePackageCount(void)
{
    if (catalogueCount == 0)
        marketplaceInit();
    return catalogueCount;
}

const MarketplacePackage *marketplaceGetPackage(int index)
{
    if (catalogueCount == 0)
        marketplaceInit();
    if (index >= 0 && index < catalogueCount)
        return &catalogue[index];
    return NULL;
}

const MarketplacePackage *marketplaceFindById(const char *packageId)
{
    if (!packageId)
        return NULL;
    if (catalogueCount == 0)
        marketplaceInit();

    for (int i = 0; i < catalogueCount; i++)
    {
        if (strcasecmp(catalogue[i].id, packageId) == 0)
            return &catalogue[i];
    }
    return NULL;
}

static int strCaseContains(const char *haystack, const char *needle)
{
    if (!needle || needle[0] == '\0')
        return 1;
    if (!haystack)
        return 0;

    size_t needleLen = strlen(needle);
    size_t haystackLen = strlen(haystack);
    if (needleLen > haystackLen)
        return 0;

    for (size_t i = 0; i <= haystackLen - needleLen; i++)
    {
        if (strncasecmp(haystack + i, needle, needleLen) == 0)
            return 1;
    }
    return 0;
}

int marketplaceSearch(const char *query, const char *category, MarketplacePackage *outResults, int maxResults, int *outCount)
{
    if (catalogueCount == 0)
        marketplaceInit();
    if (!outResults || maxResults <= 0)
        return 0;

    int count = 0;
    int checkCat = (category && category[0] && strcasecmp(category, "All") != 0);

    for (int i = 0; i < catalogueCount && count < maxResults; i++)
    {
        MarketplacePackage *p = &catalogue[i];

        if (checkCat && strcasecmp(p->category, category) != 0)
            continue;

        if (query && query[0])
        {
            if (!strCaseContains(p->name, query) &&
                !strCaseContains(p->description, query) &&
                !strCaseContains(p->id, query))
            {
                continue;
            }
        }

        outResults[count++] = *p;
    }

    if (outCount)
        *outCount = count;
    return count;
}

int marketplaceInstall(const char *packageId)
{
    MarketplacePackage *p = (MarketplacePackage *)marketplaceFindById(packageId);
    if (!p)
        return 0;

    p->isInstalled = 1;
    snprintf(p->installedVersion, sizeof(p->installedVersion), "%s", p->version);
    p->hasUpdate = 0;
    p->downloads++;

    char msg[256];
    snprintf(msg, sizeof(msg), "Extension '%s' v%s installed successfully", p->name, p->version);
    editorSetStatusMessage(msg);
    return 1;
}

int marketplaceUninstall(const char *packageId)
{
    MarketplacePackage *p = (MarketplacePackage *)marketplaceFindById(packageId);
    if (!p)
        return 0;

    p->isInstalled = 0;
    p->installedVersion[0] = '\0';
    p->hasUpdate = 0;

    char msg[256];
    snprintf(msg, sizeof(msg), "Extension '%s' uninstalled", p->name);
    editorSetStatusMessage(msg);
    return 1;
}

int marketplaceUpdate(const char *packageId)
{
    MarketplacePackage *p = (MarketplacePackage *)marketplaceFindById(packageId);
    if (!p)
        return 0;

    p->isInstalled = 1;
    snprintf(p->installedVersion, sizeof(p->installedVersion), "%s", p->version);
    p->hasUpdate = 0;

    char msg[256];
    snprintf(msg, sizeof(msg), "Updated '%s' to latest v%s", p->name, p->version);
    editorSetStatusMessage(msg);
    return 1;
}

int marketplaceCheckUpdates(int *outUpdatesCount)
{
    if (catalogueCount == 0)
        marketplaceInit();

    int updates = 0;
    for (int i = 0; i < catalogueCount; i++)
    {
        MarketplacePackage *p = &catalogue[i];
        if (p->isInstalled && p->installedVersion[0] &&
            strcmp(p->installedVersion, p->version) != 0)
        {
            p->hasUpdate = 1;
            updates++;
        }
        else
        {
            p->hasUpdate = 0;
        }
    }

    if (outUpdatesCount)
        *outUpdatesCount = updates;
    return updates;
}
