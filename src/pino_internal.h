/*
 * libpino - pino_internal.h
 * 
 */

#ifndef PINO_INTERNAL_H
#define PINO_INTERNAL_H

#include <stdio.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef PINO_SUPPLIMENTS
# include <stdio.h>
#endif

#include <pino.h>
#include <pino/handler.h>
#include <pino/endianness.h>

#define HANDLER_STEP        8
#define MM_STEP             16

#define PINO_VERSION_ID 10000000

#ifndef PINO_BUILDTIME
# define PINO_BUILDTIME 0
#endif

#define pmemcpy(dest, src, size)            memcpy(dest, src, size)
#define pmemcpy_n2l(dest, src, size)        pino_endianness_memcpy_native2le(dest, src, size)
#define pmemcpy_n2b(dest, src, size)        pino_endianness_memcpy_native2be(dest, src, size)
#define pmemcpy_l2n(dest, src, size)        pino_endianness_memcpy_le2native(dest, src, size)
#define pmemcpy_b2n(dest, src, size)        pino_endianness_memcpy_be2native(dest, src, size)
#define pmemmove(dest, src, size)           memmove(dest, src, size)
#define pmemcmp(s1, s2, size)               memcmp(s1, s2, size)
#define pmalloc(size)                       malloc(size)
#define pcalloc(count, size)                calloc(count, size)
#define prealloc(ptr, size)                 realloc(ptr, size)
#define pfree(ptr)                          free(ptr)

typedef struct {
    size_t usage;
    size_t capacity;
    void **ptrs;
} mm_t;

typedef struct {
    pino_magic_t magic;
    mm_t mm;
    pino_handler_t *handler;
} handler_entry_t;

static inline bool validate_magic(pino_magic_safe_t magic)
{
    if (!magic) {
        return false;
    }

    return 
        (strspn(
            (const char *)magic,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
        ) == sizeof(pino_magic_t)) &&
        strlen((const char *)magic) == sizeof(pino_magic_t);
}

static inline bool magic_equal(pino_magic_t magic, pino_magic_safe_t smagic)
{
    return strncmp(magic, smagic, sizeof(pino_magic_t)) == 0;
}

bool pino_handler_init(size_t initialize_size);
void pino_handler_free(void);
pino_handler_t *pino_handler_find(pino_magic_safe_t magic);

bool pino_memory_manager_obj_init(mm_t *mm, size_t initialize_size);
void pino_memory_manager_obj_free(mm_t *mm);

/* for debugging */
#ifdef PINO_SUPPLIMENTS
# define PINO_SUPRTF(fmt, ...)              printf("  %s > " fmt "\n", __func__, ##__VA_ARGS__)
# define PINO_SUPUNREACH()                  printf("  %s > !!!UNREACHABLE!!!\n", __func__)
#else
# define PINO_SUPRTF(fmt, ...)              ((void)0)
# define PINO_SUPUNREACH()                  ((void)0)
#endif

#endif  /* PINO_INTERNAL_H */
