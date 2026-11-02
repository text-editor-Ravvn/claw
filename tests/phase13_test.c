/* Claw Phase 13 Test Suite — Claw Ecosystem (Marketplace & Packaging) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "marketplace.h"
#include "package.h"
#include "viewport.h"
#include "editor.h"
#include "input.h"

/* --- Mock Editor State --- */
Viewport viewport = {24, 80, 0, 0, 0};
static char lastStatusMsg[256] = "";

void editorSetStatusMessage(const char *msg)
{
    if (msg)
        snprintf(lastStatusMsg, sizeof(lastStatusMsg), "%s", msg);
    else
        lastStatusMsg[0] = '\0';
}

const char *editorStatusMessage(void)
{
    return lastStatusMsg;
}

void renderForceClear(void)
{
}

/* ========================================================================= */
/* Test Cases                                                                */
/* ========================================================================= */

static void test_sha256_cryptographic_engine(void)
{
    printf("[1/13] Testing SHA-256 Hash Engine (Known Vector Verification)...\n");

    char hash[65];

    /* Test Vector 1: Empty string -> e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 */
    int ok = packageComputeSha256((const unsigned char *)"", 0, hash);
    assert(ok == 1);
    assert(strcmp(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0);

    /* Test Vector 2: "abc" -> ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad */
    ok = packageComputeSha256((const unsigned char *)"abc", 3, hash);
    assert(ok == 1);
    assert(strcmp(hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);

    /* Test File Hashing */
    FILE *fp = fopen(".claw_hash_test.tmp", "w");
    assert(fp != NULL);
    fputs("abc", fp);
    fclose(fp);

    ok = packageComputeFileSha256(".claw_hash_test.tmp", hash);
    assert(ok == 1);
    assert(strcmp(hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
    remove(".claw_hash_test.tmp");

    printf("  -> SHA-256 Engine tests PASSED.\n");
}

static void test_hmac_digital_signatures(void)
{
    printf("[2/13] Testing HMAC-SHA256 Digital Signature Generation & Verification...\n");

    const char *payload = "{\"package\":\"claw-theme\",\"version\":\"1.0.0\"}";
    const char *secret = "claw-community-root-key-2026";
    const char *wrongSecret = "unauthorized-private-key";

    char signature[65];
    int ok = packageSign(payload, strlen(payload), secret, signature, sizeof(signature));
    assert(ok == 1);
    assert(strlen(signature) == 64);

    /* Valid verification */
    assert(packageVerifySignature(payload, strlen(payload), signature, secret) == 1);

    /* Rejection with incorrect secret */
    assert(packageVerifySignature(payload, strlen(payload), signature, wrongSecret) == 0);

    /* Rejection with tampered payload */
    const char *tampered = "{\"package\":\"claw-theme\",\"version\":\"2.0.0\"}";
    assert(packageVerifySignature(tampered, strlen(tampered), signature, secret) == 0);

    /* Rejection with corrupted signature */
    char badSig[65];
    snprintf(badSig, sizeof(badSig), "%s", signature);
    badSig[0] = (badSig[0] == 'a') ? 'b' : 'a';
    assert(packageVerifySignature(payload, strlen(payload), badSig, secret) == 0);

    printf("  -> HMAC Digital Signatures tests PASSED.\n");
}

static void test_package_manifest_parser(void)
{
    printf("[3/13] Testing Package Manifest JSON Parser...\n");

    const char *manifestJson =
        "{\n"
        "  \"name\": \"claw-nord-theme\",\n"
        "  \"version\": \"1.2.0\",\n"
        "  \"description\": \"Nordic theme for Claw editor\",\n"
        "  \"author\": \"Arctic\",\n"
        "  \"license\": \"MIT\",\n"
        "  \"claw_version_min\": \"2.0\",\n"
        "  \"entry_point\": \"theme.lua\",\n"
        "  \"permissions\": \"fs_read\",\n"
        "  \"checksum\": \"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\",\n"
        "  \"signature\": \"valid_sig_hex_12345\"\n"
        "}\n";

    PackageManifest mf;
    int parsed = packageParseManifest(manifestJson, &mf);
    assert(parsed == 1);
    assert(strcmp(mf.name, "claw-nord-theme") == 0);
    assert(strcmp(mf.version, "1.2.0") == 0);
    assert(strcmp(mf.description, "Nordic theme for Claw editor") == 0);
    assert(strcmp(mf.author, "Arctic") == 0);
    assert(strcmp(mf.license, "MIT") == 0);
    assert(strcmp(mf.clawVersionMin, "2.0") == 0);
    assert(strcmp(mf.entryPoint, "theme.lua") == 0);
    assert(strcmp(mf.permissions, "fs_read") == 0);
    assert(strcmp(mf.checksum, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855") == 0);
    assert(strcmp(mf.signature, "valid_sig_hex_12345") == 0);

    printf("  -> Package Manifest Parser tests PASSED.\n");
}

static void test_package_manifest_validator(void)
{
    printf("[4/13] Testing Package Manifest Validator...\n");

    char err[128];
    PackageManifest mf = {0};

    /* Missing name */
    assert(packageValidateManifest(&mf, err, sizeof(err)) == 0);
    assert(strstr(err, "name") != NULL);

    /* Missing version */
    snprintf(mf.name, sizeof(mf.name), "my-plugin");
    assert(packageValidateManifest(&mf, err, sizeof(err)) == 0);
    assert(strstr(err, "version") != NULL);

    /* Missing entryPoint */
    snprintf(mf.version, sizeof(mf.version), "1.0.0");
    assert(packageValidateManifest(&mf, err, sizeof(err)) == 0);
    assert(strstr(err, "entry_point") != NULL);

    /* Complete valid manifest */
    snprintf(mf.entryPoint, sizeof(mf.entryPoint), "init.claw");
    assert(packageValidateManifest(&mf, err, sizeof(err)) == 1);

    printf("  -> Package Manifest Validator tests PASSED.\n");
}

static void test_package_manifest_formatter(void)
{
    printf("[5/13] Testing Package Manifest Serialization...\n");

    PackageManifest mf = {0};
    snprintf(mf.name, sizeof(mf.name), "claw-rust-analyzer");
    snprintf(mf.version, sizeof(mf.version), "1.8.0");
    snprintf(mf.description, sizeof(mf.description), "Rust language support");
    snprintf(mf.author, sizeof(mf.author), "Rust Team");
    snprintf(mf.entryPoint, sizeof(mf.entryPoint), "plugin.lua");

    char json[2048];
    assert(packageFormatManifest(&mf, json, sizeof(json)) > 0);
    assert(strstr(json, "\"name\": \"claw-rust-analyzer\"") != NULL);
    assert(strstr(json, "\"version\": \"1.8.0\"") != NULL);
    assert(strstr(json, "\"entry_point\": \"plugin.lua\"") != NULL);

    /* Verify round-trip parsing */
    PackageManifest roundtrip;
    assert(packageParseManifest(json, &roundtrip) == 1);
    assert(strcmp(roundtrip.name, mf.name) == 0);
    assert(strcmp(roundtrip.version, mf.version) == 0);

    printf("  -> Package Manifest Serialization tests PASSED.\n");
}

static void test_package_archive_bundling(void)
{
    printf("[6/13] Testing Package Archive Bundling (.cpkg) & Signature Verification...\n");

    const char *archivePath = ".claw_test_package.cpkg";
    const char *secretKey = "claw-community-auth-key";

    PackageManifest mf = {0};
    snprintf(mf.name, sizeof(mf.name), "claw-sample-ext");
    snprintf(mf.version, sizeof(mf.version), "1.0.0");
    snprintf(mf.description, sizeof(mf.description), "Sample Claw extension");
    snprintf(mf.author, sizeof(mf.author), "Community");
    snprintf(mf.entryPoint, sizeof(mf.entryPoint), "main.lua");

    /* Create signed archive */
    assert(packageCreateArchive(".", &mf, secretKey, archivePath) == 1);

    /* Verify file exists and has size */
    FILE *fp = fopen(archivePath, "rb");
    assert(fp != NULL);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    assert(size > 100);
    fclose(fp);

    /* Extract and verify with correct key */
    assert(packageExtractArchive(archivePath, ".claw/extracted", secretKey) == 1);

    /* Extract with wrong key must fail signature verification */
    assert(packageExtractArchive(archivePath, ".claw/extracted", "wrong-key") == 0);

    remove(archivePath);
    printf("  -> Package Archive Bundling tests PASSED.\n");
}

static void test_marketplace_catalogue_initialization(void)
{
    printf("[7/13] Testing Extension Marketplace Catalogue...\n");

    int count = marketplaceInit();
    assert(count >= 8);
    assert(marketplacePackageCount() >= 8);

    /* Verify essential packages exist */
    const MarketplacePackage *nord = marketplaceFindById("claw-nord-theme");
    assert(nord != NULL);
    assert(strcmp(nord->category, "Themes") == 0);
    assert(nord->rating == 5);
    assert(nord->downloads > 1000);

    const MarketplacePackage *clangd = marketplaceFindById("claw-clangd-lsp");
    assert(clangd != NULL);
    assert(strcmp(clangd->category, "LSP") == 0);

    const MarketplacePackage *gitlens = marketplaceFindById("claw-git-lens");
    assert(gitlens != NULL);
    assert(strcmp(gitlens->category, "Tooling") == 0);

    const MarketplacePackage *snippets = marketplaceFindById("claw-snippets-pack");
    assert(snippets != NULL);
    assert(strcmp(snippets->category, "Productivity") == 0);

    printf("  -> Marketplace Catalogue tests PASSED.\n");
}

static void test_marketplace_search_and_filter(void)
{
    printf("[8/13] Testing Marketplace Search & Category Filtering...\n");

    MarketplacePackage results[32];
    int count = 0;

    /* Category filter: Themes */
    marketplaceSearch(NULL, "Themes", results, 32, &count);
    assert(count >= 2);
    for (int i = 0; i < count; i++)
        assert(strcmp(results[i].category, "Themes") == 0);

    /* Category filter: LSP */
    marketplaceSearch(NULL, "LSP", results, 32, &count);
    assert(count >= 3);
    for (int i = 0; i < count; i++)
        assert(strcmp(results[i].category, "LSP") == 0);

    /* Text query search: "rust" */
    marketplaceSearch("rust", "All", results, 32, &count);
    assert(count >= 1);
    assert(strstr(results[0].name, "Rust") != NULL || strstr(results[0].id, "rust") != NULL);

    /* Empty query returns all */
    marketplaceSearch(NULL, "All", results, 32, &count);
    assert(count >= 8);

    printf("  -> Marketplace Search & Filtering tests PASSED.\n");
}

static void test_marketplace_installation_lifecycle(void)
{
    printf("[9/13] Testing Marketplace Package Installation Lifecycle...\n");

    const MarketplacePackage *pkg = marketplaceFindById("claw-nord-theme");
    assert(pkg != NULL);
    assert(pkg->isInstalled == 0);
    int initialDl = pkg->downloads;

    /* Install */
    int ok = marketplaceInstall("claw-nord-theme");
    assert(ok == 1);
    assert(pkg->isInstalled == 1);
    assert(strcmp(pkg->installedVersion, pkg->version) == 0);
    assert(pkg->downloads == initialDl + 1);

    printf("  -> Marketplace Installation tests PASSED.\n");
}

static void test_marketplace_update_lifecycle(void)
{
    printf("[10/13] Testing Marketplace Update Checking & Upgrading...\n");

    MarketplacePackage *pkg = (MarketplacePackage *)marketplaceFindById("claw-nord-theme");
    assert(pkg != NULL);
    assert(pkg->isInstalled == 1);

    /* Simulate older version installed */
    snprintf(pkg->installedVersion, sizeof(pkg->installedVersion), "1.0.0");

    int updatesCount = 0;
    marketplaceCheckUpdates(&updatesCount);
    assert(updatesCount >= 1);
    assert(pkg->hasUpdate == 1);

    /* Upgrade */
    assert(marketplaceUpdate("claw-nord-theme") == 1);
    assert(pkg->hasUpdate == 0);
    assert(strcmp(pkg->installedVersion, pkg->version) == 0);

    printf("  -> Marketplace Update tests PASSED.\n");
}

static void test_marketplace_uninstallation_lifecycle(void)
{
    printf("[11/13] Testing Marketplace Package Uninstallation...\n");

    const MarketplacePackage *pkg = marketplaceFindById("claw-nord-theme");
    assert(pkg != NULL);
    assert(pkg->isInstalled == 1);

    assert(marketplaceUninstall("claw-nord-theme") == 1);
    assert(pkg->isInstalled == 0);
    assert(pkg->installedVersion[0] == '\0');

    printf("  -> Marketplace Uninstallation tests PASSED.\n");
}

static void test_marketplace_ui_modal_navigation(void)
{
    printf("[12/13] Testing Marketplace UI Modal Controller & Category Navigation...\n");

    assert(marketplaceIsActive() == 0);
    marketplaceOpen();
    assert(marketplaceIsActive() == 1);
    assert(marketplaceUI.selectedCategoryIndex == 0);

    /* Cycle category via Tab */
    marketplaceHandleKey('\t', 0);
    assert(marketplaceUI.selectedCategoryIndex == 1); /* Themes */

    /* Jump directly to category via numeric keys */
    marketplaceHandleKey('5', 0);
    assert(marketplaceUI.selectedCategoryIndex == 4); /* LSP */

    /* Navigate package list */
    marketplaceHandleKey(ARROW_DOWN, 0);
    assert(marketplaceUI.selectedIndex == 1);
    marketplaceHandleKey(ARROW_UP, 0);
    assert(marketplaceUI.selectedIndex == 0);

    /* Exit */
    marketplaceHandleKey(27, 0);
    assert(marketplaceIsActive() == 0);

    printf("  -> Marketplace UI Navigation tests PASSED.\n");
}

static void test_marketplace_ui_search_input(void)
{
    printf("[13/13] Testing Marketplace Search Input Mode...\n");

    marketplaceOpen();
    assert(marketplaceUI.searching == 0);

    /* Enter search mode with '/' */
    marketplaceHandleKey('/', 0);
    assert(marketplaceUI.searching == 1);

    /* Type "pyright" */
    marketplaceHandleKey('p', 0);
    marketplaceHandleKey('y', 0);
    assert(strcmp(marketplaceUI.searchQuery, "py") == 0);

    /* Exit search mode */
    marketplaceHandleKey('\r', 0);
    assert(marketplaceUI.searching == 0);
    assert(strcmp(marketplaceUI.searchQuery, "py") == 0);

    /* Close */
    marketplaceHandleKey(27, 0);
    assert(marketplaceIsActive() == 0);

    printf("  -> Marketplace Search Input tests PASSED.\n");
}

int main(void)
{
    printf("\n==================================================\n");
    printf("   Claw Phase 13 Test Suite (Marketplace & Ecosystem)\n");
    printf("==================================================\n");

    test_sha256_cryptographic_engine();
    test_hmac_digital_signatures();
    test_package_manifest_parser();
    test_package_manifest_validator();
    test_package_manifest_formatter();
    test_package_archive_bundling();
    test_marketplace_catalogue_initialization();
    test_marketplace_search_and_filter();
    test_marketplace_installation_lifecycle();
    test_marketplace_update_lifecycle();
    test_marketplace_uninstallation_lifecycle();
    test_marketplace_ui_modal_navigation();
    test_marketplace_ui_search_input();

    printf("\n==================================================\n");
    printf("   ALL PHASE 13 TESTS COMPLETED & PASSED!\n");
    printf("==================================================\n\n");

    return 0;
}
