/*
 * libpino tests - util.h
 * 
 */

#ifndef PINO_TESTS_UTIL_H
#define PINO_TESTS_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#if defined(__linux__)
# include <linux/limits.h>
#elif defined(__APPLE__)
# include <sys/syslimits.h>
#elif defined(_WIN32)
# include <windows.h>
# define PATH_MAX MAX_PATH
#endif

static inline bool get_asset_path(char *filename, char *path, size_t path_size)
{
    size_t required_size;

    if (path == NULL || path_size == 0) {
        return false;
    }

    memset(path, 0, path_size);

#ifdef PINO_TEST_ASSETS_DIR
    required_size = strlen(PINO_TEST_ASSETS_DIR) + strlen(filename) + 2;
#else
    required_size = strlen("assets/") + strlen(filename) + 1;
#endif

    if (required_size > path_size) {
        return false;
    }

#ifdef PINO_TEST_ASSETS_DIR
    snprintf(path, path_size, "%s/%s", PINO_TEST_ASSETS_DIR, filename);
#else
    snprintf(path, path_size, "assets/%s", filename);
#endif

    return true;
}

static inline bool load_file(const char *filename, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    size_t readsize;
    long size;

    file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    size = ftell(file);
    if (size < 0) {
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    *out_data = (uint8_t*)malloc(size);
    if (!*out_data) {
        fclose(file);
        return false;
    }

    readsize = fread(*out_data, 1, size, file);
    if (fclose(file) != 0) {
        free(*out_data);
        *out_data = NULL;
        return false;
    }

    if (readsize != (size_t)size) {
        free(*out_data);
        *out_data = NULL;
        return false;
    }

    *out_size = size;
    return true;
}

static inline bool save_file(const char *filename, const uint8_t *data, size_t size)
{
    FILE *file;
    size_t written;

    file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

    written = fwrite(data, 1, size, file);
    if (fclose(file) != 0 || written != size) {
        return false;
    }

    return true;
}

static inline void generate_random_str(uint8_t *out, size_t size)
{
    size_t i;
    const uint8_t charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charset_size = sizeof(charset) - 1;

    if (size > 0) {
        for (i = 0; i < size - 1; i++) {
            out[i] = charset[rand() % charset_size];
        }
        out[size - 1] = '\0';
    }
}

static inline void generate_fixed_data(uint8_t *out, size_t size)
{
    size_t i;

    for (i = 0; i < size; i++) {
        out[i] = (uint8_t)(i % UINT8_MAX);
    }
}

static inline void generate_random_data(uint8_t *out, size_t size)
{
    size_t i;

    srand((unsigned int)time(NULL));

    for (i = 0; i < size; i++) {
        out[i] = (uint8_t)(rand() % 256);
    }
}

static inline bool break_data(uint8_t *data, size_t data_size, uint32_t count)
{
    uint32_t corrupted_count;
    size_t pos;

    if (!data || data_size == 0 || count == 0) {
        return false;
    }

    if (count > data_size) {
        count = data_size;
    }

    bool *corrupted = (bool *)calloc(data_size, sizeof(bool));
    if (!corrupted) {
        return false;
    }

    srand((unsigned int)time(NULL));

    corrupted_count = 0;
    while (corrupted_count < count) {
        pos = rand() % data_size;
        pos %= data_size;

        if (!corrupted[pos]) {
            data[pos] ^= 0xFF;
            corrupted[pos] = true;
            corrupted_count++;
        }
    }

    free(corrupted);

    return true;
}

static inline void print_hex(uint8_t *data, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

#endif  /* PINO_TESTS_UTIL_H */
