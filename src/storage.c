/* ==============================================================================
 * Project: 2048-Core
 * File: src/storage.c
 * Standard: ANSI C (C89)
 * ============================================================================== */

#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File Format Constants */
/* 0x474D324B = 'G', 'M', '2', 'K' */
static const unsigned long MAGIC_NUMBER = 0x474D324B;
/* Bumped version to 2 to support GameState struct change (added high_score) */
static const unsigned char FILE_VERSION = 2;
static const char *FILENAME = "save.dat";
static const char *TMP_FILENAME = "save.tmp";

/* * Helper: Simple Checksum (Adler-32 style simplification suitable for C89)
 * We treat the struct as a raw byte array.
 */
static unsigned long calculate_checksum(const GameState *state)
{
    const unsigned char *data = (const unsigned char *)state;
    unsigned long s1 = 1;
    unsigned long s2 = 0;
    size_t i;
    size_t len = sizeof(GameState);

    /* Prime modulo arithmetic to avoid overflow patterns */
    for (i = 0; i < len; i++) {
        s1 = (s1 + data[i]) % 65521;
        s2 = (s2 + s1) % 65521;
    }

    return (s2 << 16) | s1;
}

/* * Helper: Path Construction
 * Concatenates base_path and filename into dest.
 * Assumes dest is large enough (512 bytes recommended).
 */
static void build_path(char *dest, const char *base, const char *file)
{
    strcpy(dest, base);
    /* Add separator if missing and base is not empty */
    if (base[0] != '\0' && base[strlen(base) - 1] != '/') {
        strcat(dest, "/");
    }
    strcat(dest, file);
}

int storage_save(const char *base_path, const GameState *state)
{
    FILE *fp;
    char tmp_path[512];
    char final_path[512];
    unsigned long checksum;
    unsigned long magic = MAGIC_NUMBER;
    unsigned char ver = FILE_VERSION;

    /* 1. Calculate Checksum before IO */
    checksum = calculate_checksum(state);

    /* 2. Construct Paths */
    build_path(tmp_path, base_path, TMP_FILENAME);
    build_path(final_path, base_path, FILENAME);

    /* 3. Open Temporary File */
    fp = fopen(tmp_path, "wb");
    if (!fp)
        return STORAGE_ERR_OPEN;

    /* 4. Write Header */
    if (fwrite(&magic, sizeof(magic), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_WRITE;
    }
    if (fwrite(&ver, sizeof(ver), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_WRITE;
    }

    /* 5. Write Payload (Handles new high_score automatically via sizeof) */
    if (fwrite(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_WRITE;
    }

    /* 6. Write Checksum */
    if (fwrite(&checksum, sizeof(checksum), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_WRITE;
    }

    /* 7. Close and Commit */
    fclose(fp);

    /* Atomic Switch: Remove old, Rename new */
    /* Note: 'rename' behavior on existing targets is implementation-defined,
       so we explicitly remove the target first for cross-platform C89 safety. */
    remove(final_path);
    if (rename(tmp_path, final_path) != 0) {
        return STORAGE_ERR_WRITE;
    }

    return STORAGE_OK;
}

int storage_load(const char *base_path, GameState *state)
{
    FILE *fp;
    char final_path[512];
    unsigned long magic_in, checksum_in, checksum_calc;
    unsigned char ver_in;

    /* Safety: Zero out state first.
     * This ensures high_score is 0 if loading fails or version is mismatched.
     */
    memset(state, 0, sizeof(GameState));

    build_path(final_path, base_path, FILENAME);

    /* 1. Open File */
    fp = fopen(final_path, "rb");
    if (!fp)
        return STORAGE_ERR_NOT_FOUND;

    /* 2. Verify Magic Number */
    if (fread(&magic_in, sizeof(magic_in), 1, fp) != 1 || magic_in != MAGIC_NUMBER) {
        fclose(fp);
        return STORAGE_ERR_CORRUPT;
    }

    /* 3. Verify Version */
    if (fread(&ver_in, sizeof(ver_in), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_CORRUPT;
    }
    if (ver_in != FILE_VERSION) {
        fclose(fp);
        /* Rejecting old versions ensures we don't load corrupt/misaligned data
         * into the new struct layout. Caller will init a fresh game. */
        return STORAGE_ERR_VERSION;
    }

    /* 4. Read Payload */
    if (fread(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_CORRUPT;
    }

    /* 5. Verify Checksum */
    if (fread(&checksum_in, sizeof(checksum_in), 1, fp) != 1) {
        fclose(fp);
        return STORAGE_ERR_CORRUPT;
    }

    fclose(fp);

    /* 6. Validate Data Integrity */
    checksum_calc = calculate_checksum(state);
    if (checksum_calc != checksum_in) {
        /* Zero out state again to prevent usage of corrupted data */
        memset(state, 0, sizeof(GameState));
        return STORAGE_ERR_CORRUPT;
    }

    return STORAGE_OK;
}