/* Claw Plugin & Extension Marketplace Interactive Modal (Phase 13) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "marketplace.h"
#include "editor.h"
#include "viewport.h"
#include "render.h"
#include "input.h"

static const char *categories[MARKETPLACE_CATEGORIES_COUNT] = {
    "All", "Themes", "Languages", "Tooling", "LSP", "Productivity"
};

void marketplaceOpen(void)
{
    marketplaceInit();
    marketplaceUI.active = 1;
    marketplaceUI.currentTab = 0;
    marketplaceUI.selectedCategoryIndex = 0;
    marketplaceUI.selectedIndex = 0;
    marketplaceUI.scrollOffset = 0;
    marketplaceUI.searchQuery[0] = '\0';
    marketplaceUI.queryLen = 0;
    marketplaceUI.searching = 0;
    marketplaceUI.statusMessage[0] = '\0';
    editorSetStatusMessage("");
}

void marketplaceClose(void)
{
    marketplaceUI.active = 0;
    renderForceClear();
}

int marketplaceIsActive(void)
{
    return marketplaceUI.active;
}

void marketplaceDraw(void)
{
    if (!marketplaceUI.active)
        return;

    printf("\033[2J\033[H");
    printf("\033[1;35m=== Claw Extension Marketplace ===\033[0m   ");
    if (marketplaceUI.searching)
        printf("Search: \033[1;33m%s_\033[0m\r\n", marketplaceUI.searchQuery);
    else if (marketplaceUI.queryLen > 0)
        printf("Search: \033[1;37m%s\033[0m  (press / to search)\r\n", marketplaceUI.searchQuery);
    else
        printf("\033[90m(press / to search extensions)\033[0m\r\n");

    printf("\033[90m--------------------------------------------------------------------------------\033[0m\r\n");

    /* Category bar */
    printf(" Category: ");
    for (int c = 0; c < MARKETPLACE_CATEGORIES_COUNT; c++)
    {
        if (c == marketplaceUI.selectedCategoryIndex)
            printf("\033[7m [%d] %s \033[0m ", c + 1, categories[c]);
        else
            printf(" [%d] %s  ", c + 1, categories[c]);
    }
    printf("\r\n");
    printf("\033[90m================================================================================\033[0m\r\n");

    /* Retrieve matching packages */
    MarketplacePackage results[MARKETPLACE_MAX_PACKAGES];
    int count = 0;
    marketplaceSearch(marketplaceUI.searchQuery, categories[marketplaceUI.selectedCategoryIndex],
                      results, MARKETPLACE_MAX_PACKAGES, &count);

    int maxDisplay = viewport.screenRows - 11;
    if (maxDisplay < 4) maxDisplay = 4;

    int start = 0;
    if (marketplaceUI.selectedIndex >= maxDisplay)
        start = marketplaceUI.selectedIndex - maxDisplay + 1;

    for (int i = 0; i < maxDisplay; i++)
    {
        int idx = start + i;
        if (idx >= count)
        {
            printf("\r\n");
            continue;
        }

        MarketplacePackage *p = &results[idx];
        if (idx == marketplaceUI.selectedIndex)
            printf("\033[7m");

        char stars[8] = "*****";
        if (p->rating > 0 && p->rating <= 5)
            stars[p->rating] = '\0';

        char badge[32] = "";
        if (p->isInstalled)
        {
            if (p->hasUpdate)
                snprintf(badge, sizeof(badge), "\033[1;33m[UPDATE]\033[0m");
            else
                snprintf(badge, sizeof(badge), "\033[1;32m[INSTALLED]\033[0m");
        }

        printf(" %c %-22.22s v%-6.6s %-12.12s %-4.4s %5d dl  %-11s\033[0m\r\n",
               (idx == marketplaceUI.selectedIndex) ? '>' : ' ',
               p->name, p->version, p->category, stars, p->downloads, badge);
    }

    printf("\033[90m--------------------------------------------------------------------------------\033[0m\r\n");

    /* Details Panel */
    if (count > 0 && marketplaceUI.selectedIndex < count)
    {
        MarketplacePackage *sel = &results[marketplaceUI.selectedIndex];
        printf("\033[1;36mDetails:\033[0m \033[1m%s\033[0m by %s (%s)\r\n",
               sel->name, sel->author, sel->homepage);
        printf("         %s\r\n", sel->description);
    }
    else
    {
        printf("\033[90mNo extensions found matching current filter.\033[0m\r\n\r\n");
    }

    /* Footer Hotkeys */
    printf("\033[%d;1H\033[1;35m[i] Install  [u] Update  [d] Uninstall  [Tab] Category  [/] Search  [Esc] Exit\033[0m",
           viewport.screenRows);
}

void marketplaceHandleKey(int key, int modifiers)
{
    (void)modifiers;

    if (marketplaceUI.searching)
    {
        if (key == 27 || key == '\r' || key == '\n')
        {
            marketplaceUI.searching = 0;
            return;
        }
        if (key == 127 || key == 8)
        {
            if (marketplaceUI.queryLen > 0)
            {
                marketplaceUI.searchQuery[--marketplaceUI.queryLen] = '\0';
                marketplaceUI.selectedIndex = 0;
            }
            return;
        }
        if (key >= 32 && key <= 126 && marketplaceUI.queryLen < (int)sizeof(marketplaceUI.searchQuery) - 1)
        {
            marketplaceUI.searchQuery[marketplaceUI.queryLen++] = (char)key;
            marketplaceUI.searchQuery[marketplaceUI.queryLen] = '\0';
            marketplaceUI.selectedIndex = 0;
            return;
        }
        return;
    }

    if (key == 27) /* Esc */
    {
        marketplaceClose();
        return;
    }

    if (key == '/')
    {
        marketplaceUI.searching = 1;
        return;
    }

    if (key == '\t')
    {
        marketplaceUI.selectedCategoryIndex = (marketplaceUI.selectedCategoryIndex + 1) % MARKETPLACE_CATEGORIES_COUNT;
        marketplaceUI.selectedIndex = 0;
        return;
    }

    if (key >= '1' && key <= '6')
    {
        marketplaceUI.selectedCategoryIndex = key - '1';
        marketplaceUI.selectedIndex = 0;
        return;
    }

    /* Retrieve count */
    MarketplacePackage results[MARKETPLACE_MAX_PACKAGES];
    int count = 0;
    marketplaceSearch(marketplaceUI.searchQuery, categories[marketplaceUI.selectedCategoryIndex],
                      results, MARKETPLACE_MAX_PACKAGES, &count);

    if (key == ARROW_UP || key == 'k')
    {
        if (marketplaceUI.selectedIndex > 0)
            marketplaceUI.selectedIndex--;
        return;
    }

    if (key == ARROW_DOWN || key == 'j')
    {
        if (marketplaceUI.selectedIndex < count - 1)
            marketplaceUI.selectedIndex++;
        return;
    }

    if (count > 0 && marketplaceUI.selectedIndex < count)
    {
        MarketplacePackage *sel = &results[marketplaceUI.selectedIndex];
        if (key == 'i' || key == '\r' || key == '\n')
        {
            marketplaceInstall(sel->id);
            return;
        }
        if (key == 'u')
        {
            marketplaceUpdate(sel->id);
            return;
        }
        if (key == 'd')
        {
            marketplaceUninstall(sel->id);
            return;
        }
    }
}
