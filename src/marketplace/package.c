/* Claw Package Manifest, SHA-256 Integrity & Packaging (Phase 13) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "package.h"

/* ========================================================================= */
/* Self-contained RFC 6234 SHA-256 Implementation                           */
/* ========================================================================= */

typedef struct
{
    uint32_t state[8];
    uint64_t count;
    unsigned char buffer[64];
} Sha256Context;

#define DBL_INT_ADD(a,b,c) if (a > 0xffffffff - (c)) ++b; a += c;
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void sha256_transform(Sha256Context *ctx, const unsigned char *data)
{
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = ((uint32_t)data[j] << 24) | ((uint32_t)data[j + 1] << 16) | ((uint32_t)data[j + 2] << 8) | ((uint32_t)data[j + 3]);
    for (; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; ++i)
    {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(Sha256Context *ctx)
{
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85; ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c; ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
    ctx->count = 0;
}

static void sha256_update(Sha256Context *ctx, const unsigned char *data, size_t len)
{
    size_t i = 0;
    size_t index = (size_t)(ctx->count & 63);
    ctx->count += len;

    if (index)
    {
        size_t left = 64 - index;
        if (len < left)
        {
            memcpy(ctx->buffer + index, data, len);
            return;
        }
        memcpy(ctx->buffer + index, data, left);
        sha256_transform(ctx, ctx->buffer);
        i = left;
    }

    for (; i + 64 <= len; i += 64)
        sha256_transform(ctx, data + i);

    if (i < len)
        memcpy(ctx->buffer, data + i, len - i);
}

static void sha256_final(Sha256Context *ctx, unsigned char *hash)
{
    unsigned char pad[64] = {0x80};
    uint64_t bits = ctx->count * 8;
    unsigned char bits_be[8];
    for (int i = 0; i < 8; i++)
        bits_be[i] = (unsigned char)((bits >> ((7 - i) * 8)) & 0xff);

    size_t index = (size_t)(ctx->count & 63);
    size_t padlen = (index < 56) ? (56 - index) : (120 - index);
    sha256_update(ctx, pad, padlen);
    sha256_update(ctx, bits_be, 8);

    for (int i = 0; i < 8; i++)
    {
        hash[i * 4]     = (unsigned char)((ctx->state[i] >> 24) & 0xff);
        hash[i * 4 + 1] = (unsigned char)((ctx->state[i] >> 16) & 0xff);
        hash[i * 4 + 2] = (unsigned char)((ctx->state[i] >> 8) & 0xff);
        hash[i * 4 + 3] = (unsigned char)(ctx->state[i] & 0xff);
    }
}

int packageComputeSha256(const unsigned char *data, size_t len, char *outHex)
{
    if (!outHex)
        return 0;

    Sha256Context ctx;
    sha256_init(&ctx);
    if (data && len > 0)
        sha256_update(&ctx, data, len);

    unsigned char hash[32];
    sha256_final(&ctx, hash);

    for (int i = 0; i < 32; i++)
        sprintf(outHex + (i * 2), "%02x", hash[i]);
    outHex[64] = '\0';

    return 1;
}

int packageComputeFileSha256(const char *filePath, char *outHex)
{
    if (!filePath || !outHex)
        return 0;

    FILE *fp = fopen(filePath, "rb");
    if (!fp)
        return 0;

    Sha256Context ctx;
    sha256_init(&ctx);

    unsigned char buf[4096];
    size_t bytesRead = 0;
    while ((bytesRead = fread(buf, 1, sizeof(buf), fp)) > 0)
    {
        sha256_update(&ctx, buf, bytesRead);
    }
    fclose(fp);

    unsigned char hash[32];
    sha256_final(&ctx, hash);

    for (int i = 0; i < 32; i++)
        sprintf(outHex + (i * 2), "%02x", hash[i]);
    outHex[64] = '\0';

    return 1;
}

/* --- HMAC-SHA256 Digital Signature --- */

int packageSign(const char *data, size_t dataLen, const char *secretKey, char *outSignature, size_t sigSize)
{
    if (!data || !secretKey || !outSignature || sigSize < 65)
        return 0;

    unsigned char k_pad[64];
    unsigned char tk[32];
    size_t keyLen = strlen(secretKey);

    if (keyLen > 64)
    {
        Sha256Context kctx;
        sha256_init(&kctx);
        sha256_update(&kctx, (const unsigned char *)secretKey, keyLen);
        sha256_final(&kctx, tk);
        memcpy(k_pad, tk, 32);
        memset(k_pad + 32, 0, 32);
    }
    else
    {
        memcpy(k_pad, secretKey, keyLen);
        memset(k_pad + keyLen, 0, 64 - keyLen);
    }

    /* Inner hash */
    unsigned char ipad[64];
    for (int i = 0; i < 64; i++)
        ipad[i] = k_pad[i] ^ 0x36;

    Sha256Context in_ctx;
    sha256_init(&in_ctx);
    sha256_update(&in_ctx, ipad, 64);
    sha256_update(&in_ctx, (const unsigned char *)data, dataLen);
    unsigned char innerHash[32];
    sha256_final(&in_ctx, innerHash);

    /* Outer hash */
    unsigned char opad[64];
    for (int i = 0; i < 64; i++)
        opad[i] = k_pad[i] ^ 0x5c;

    Sha256Context out_ctx;
    sha256_init(&out_ctx);
    sha256_update(&out_ctx, opad, 64);
    sha256_update(&out_ctx, innerHash, 32);
    unsigned char hmac[32];
    sha256_final(&out_ctx, hmac);

    for (int i = 0; i < 32; i++)
        sprintf(outSignature + (i * 2), "%02x", hmac[i]);
    outSignature[64] = '\0';

    return 1;
}

int packageVerifySignature(const char *data, size_t dataLen, const char *signature, const char *secretKey)
{
    if (!data || !signature || !secretKey)
        return 0;

    char expectedSig[65];
    if (!packageSign(data, dataLen, secretKey, expectedSig, sizeof(expectedSig)))
        return 0;

    return (strcmp(expectedSig, signature) == 0);
}

/* --- Manifest Parser & Formatter --- */

static void extractJsonStringField(const char *json, const char *field, char *out, size_t outSize)
{
    if (!json || !field || !out || outSize == 0)
        return;

    out[0] = '\0';
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", field);
    const char *pos = strstr(json, pattern);
    if (!pos)
        return;

    pos += strlen(pattern);
    while (*pos == ' ' || *pos == '\t' || *pos == ':')
        pos++;

    if (*pos == '\"')
    {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '\"' && i < outSize - 1)
        {
            if (*pos == '\\' && *(pos + 1))
                pos++;
            out[i++] = *pos++;
        }
        out[i] = '\0';
    }
}

int packageParseManifest(const char *jsonStr, PackageManifest *outManifest)
{
    if (!jsonStr || !outManifest)
        return 0;

    memset(outManifest, 0, sizeof(*outManifest));

    extractJsonStringField(jsonStr, "name", outManifest->name, sizeof(outManifest->name));
    extractJsonStringField(jsonStr, "version", outManifest->version, sizeof(outManifest->version));
    extractJsonStringField(jsonStr, "description", outManifest->description, sizeof(outManifest->description));
    extractJsonStringField(jsonStr, "author", outManifest->author, sizeof(outManifest->author));
    extractJsonStringField(jsonStr, "license", outManifest->license, sizeof(outManifest->license));
    extractJsonStringField(jsonStr, "claw_version_min", outManifest->clawVersionMin, sizeof(outManifest->clawVersionMin));
    extractJsonStringField(jsonStr, "entry_point", outManifest->entryPoint, sizeof(outManifest->entryPoint));
    extractJsonStringField(jsonStr, "permissions", outManifest->permissions, sizeof(outManifest->permissions));
    extractJsonStringField(jsonStr, "checksum", outManifest->checksum, sizeof(outManifest->checksum));
    extractJsonStringField(jsonStr, "signature", outManifest->signature, sizeof(outManifest->signature));

    return (outManifest->name[0] != '\0' && outManifest->version[0] != '\0');
}

int packageValidateManifest(const PackageManifest *manifest, char *errBuf, size_t errBufSize)
{
    if (!manifest)
    {
        if (errBuf && errBufSize > 0)
            snprintf(errBuf, errBufSize, "Manifest is null");
        return 0;
    }

    if (manifest->name[0] == '\0')
    {
        if (errBuf && errBufSize > 0)
            snprintf(errBuf, errBufSize, "Missing required field: 'name'");
        return 0;
    }

    if (manifest->version[0] == '\0')
    {
        if (errBuf && errBufSize > 0)
            snprintf(errBuf, errBufSize, "Missing required field: 'version'");
        return 0;
    }

    if (manifest->entryPoint[0] == '\0')
    {
        if (errBuf && errBufSize > 0)
            snprintf(errBuf, errBufSize, "Missing required field: 'entry_point'");
        return 0;
    }

    return 1;
}

int packageFormatManifest(const PackageManifest *manifest, char *outJson, size_t outSize)
{
    if (!manifest || !outJson || outSize == 0)
        return 0;

    int written = snprintf(outJson, outSize,
        "{\n"
        "  \"name\": \"%s\",\n"
        "  \"version\": \"%s\",\n"
        "  \"description\": \"%s\",\n"
        "  \"author\": \"%s\",\n"
        "  \"license\": \"%s\",\n"
        "  \"claw_version_min\": \"%s\",\n"
        "  \"entry_point\": \"%s\",\n"
        "  \"permissions\": \"%s\",\n"
        "  \"checksum\": \"%s\",\n"
        "  \"signature\": \"%s\"\n"
        "}\n",
        manifest->name, manifest->version, manifest->description,
        manifest->author, manifest->license[0] ? manifest->license : "MIT",
        manifest->clawVersionMin[0] ? manifest->clawVersionMin : "2.0",
        manifest->entryPoint, manifest->permissions, manifest->checksum, manifest->signature);

    return (written > 0 && (size_t)written < outSize) ? written : 0;
}

/* --- Archive Creation & Extraction --- */

int packageCreateArchive(const char *sourceDir, const PackageManifest *manifest, const char *secretKey, const char *outputPackageFile)
{
    if (!manifest || !outputPackageFile)
        return 0;

    (void)sourceDir;

    /* Compute payload checksum & signature */
    PackageManifest finalManifest = *manifest;
    char manifestJson[2048];
    packageFormatManifest(&finalManifest, manifestJson, sizeof(manifestJson));

    packageComputeSha256((const unsigned char *)manifestJson, strlen(manifestJson), finalManifest.checksum);
    if (secretKey)
    {
        packageSign(finalManifest.checksum, strlen(finalManifest.checksum), secretKey, finalManifest.signature, sizeof(finalManifest.signature));
    }

    FILE *fp = fopen(outputPackageFile, "wb");
    if (!fp)
        return 0;

    /* Archive format: Magic "CPKG" + 4-byte manifest len + Manifest JSON + dummy embedded files */
    const char magic[4] = {'C', 'P', 'K', 'G'};
    fwrite(magic, 1, 4, fp);

    packageFormatManifest(&finalManifest, manifestJson, sizeof(manifestJson));
    uint32_t len = (uint32_t)strlen(manifestJson);
    fwrite(&len, sizeof(uint32_t), 1, fp);
    fwrite(manifestJson, 1, len, fp);

    /* Dummy entry point payload */
    const char *payload = "-- Claw Plugin Entrypoint\nprint(\"Loaded extension: " ;
    fwrite(payload, 1, strlen(payload), fp);
    fwrite(finalManifest.name, 1, strlen(finalManifest.name), fp);
    fwrite("\")\n", 1, 3, fp);

    fclose(fp);
    return 1;
}

int packageExtractArchive(const char *packageFile, const char *targetDir, const char *expectedSecretKey)
{
    if (!packageFile || !targetDir)
        return 0;

    FILE *fp = fopen(packageFile, "rb");
    if (!fp)
        return 0;

    char magic[4];
    if (fread(magic, 1, 4, fp) != 4 || memcmp(magic, "CPKG", 4) != 0)
    {
        fclose(fp);
        return 0;
    }

    uint32_t manifestLen = 0;
    if (fread(&manifestLen, sizeof(uint32_t), 1, fp) != 1 || manifestLen > 65536)
    {
        fclose(fp);
        return 0;
    }

    char *manifestJson = malloc(manifestLen + 1);
    if (!manifestJson)
    {
        fclose(fp);
        return 0;
    }

    if (fread(manifestJson, 1, manifestLen, fp) != manifestLen)
    {
        free(manifestJson);
        fclose(fp);
        return 0;
    }
    manifestJson[manifestLen] = '\0';

    PackageManifest mf;
    if (!packageParseManifest(manifestJson, &mf))
    {
        free(manifestJson);
        fclose(fp);
        return 0;
    }
    free(manifestJson);

    /* Verify signature if requested */
    if (expectedSecretKey && mf.signature[0])
    {
        if (!packageVerifySignature(mf.checksum, strlen(mf.checksum), mf.signature, expectedSecretKey))
        {
            fclose(fp);
            return 0; /* Signature verification failed! */
        }
    }

    fclose(fp);
    return 1;
}
