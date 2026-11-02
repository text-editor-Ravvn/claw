/* Claw Package Manifest, Signature Verification & Packaging Header (Phase 13) */
#ifndef PACKAGE_H
#define PACKAGE_H

#include <stddef.h>

#define PACKAGE_MAX_FILES 64
#define PACKAGE_MANIFEST_FILE "claw-package.json"

/* Package Manifest */
typedef struct
{
    char name[64];
    char version[32];
    char description[256];
    char author[64];
    char license[32];
    char clawVersionMin[32];
    char entryPoint[64];      /* e.g. "plugin.lua" or "init.claw" */
    char permissions[128];     /* comma-separated permissions e.g. "fs_read,process_exec" */
    char checksum[65];        /* hex SHA-256 */
    char signature[128];      /* HMAC-SHA256 signature */
} PackageManifest;

/* --- Manifest Parser & Validator --- */
int packageParseManifest(const char *jsonStr, PackageManifest *outManifest);
int packageValidateManifest(const PackageManifest *manifest, char *errBuf, size_t errBufSize);
int packageFormatManifest(const PackageManifest *manifest, char *outJson, size_t outSize);

/* --- Cryptographic Integrity & Signature Verification --- */
int packageComputeSha256(const unsigned char *data, size_t len, char *outHex);
int packageComputeFileSha256(const char *filePath, char *outHex);
int packageSign(const char *data, size_t dataLen, const char *secretKey, char *outSignature, size_t sigSize);
int packageVerifySignature(const char *data, size_t dataLen, const char *signature, const char *secretKey);

/* --- Packaging & Extraction Utilities --- */
int packageCreateArchive(const char *sourceDir, const PackageManifest *manifest, const char *secretKey, const char *outputPackageFile);
int packageExtractArchive(const char *packageFile, const char *targetDir, const char *expectedSecretKey);

#endif /* PACKAGE_H */
