/*
 * libpino - endianness.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <pino/endianness.h>
#include <pino/portable.h>

#include "internal/bswap.h"
#include "internal/common.h"

#define ENDIANNESS_UNKNOWN 0
#define ENDIANNESS_LITTLE  1
#define ENDIANNESS_BIG     2

#define bswap_memcpy pino_bswap_memcpy

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static uint8_t g_endianness = ENDIANNESS_LITTLE;
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static uint8_t g_endianness = ENDIANNESS_BIG;
#else
static uint8_t g_endianness = ENDIANNESS_UNKNOWN;
#endif

static inline uint8_t platform_endianness(void)
{
    uint32_t i = 1;
    uint8_t *p = (uint8_t *)&i;

    if (g_endianness != ENDIANNESS_UNKNOWN) {
        return g_endianness;
    }

    return g_endianness = (p[0] == 1) ? ENDIANNESS_LITTLE : ENDIANNESS_BIG;
}

static inline size_t normalize_elem_size(size_t elem_size)
{
    return elem_size == 0 ? 1 : elem_size;
}

static inline void *memcpy_common(void *dest, const void *src, size_t size, size_t elem_size, bool is_native)
{
    return is_native ? pmemcpy(dest, src, size) : bswap_memcpy(dest, src, size, normalize_elem_size(elem_size));
}

static inline void *memmove_common(void *dest, const void *src, size_t size, size_t elem_size, bool is_native)
{
    uint8_t *tmp;

    if (is_native) {
        return pmemmove(dest, src, size);
    }

    tmp = (uint8_t *)pmalloc(size);
    if (tmp == NULL) {
        return NULL;
    }

    pmemcpy(tmp, src, size);
    bswap_memcpy(dest, tmp, size, normalize_elem_size(elem_size));

    pfree(tmp);

    return dest;
}

static inline int memcmp_common(const void *s1, const void *s2, size_t size, size_t elem_size, bool is_native)
{
    uint8_t *tmp1, *tmp2;
    int result;

    if (is_native) {
        return pmemcmp(s1, s2, size);
    }

    tmp1 = (uint8_t *)pmalloc(size);
    tmp2 = (uint8_t *)pmalloc(size);
    if (!tmp1 || !tmp2) {
        if (tmp1) {
            pfree(tmp1);
        }
        if (tmp2) {
            pfree(tmp2);
        }
        return PINO_MEMCMP_ERROR;
    }

    elem_size = normalize_elem_size(elem_size);
    bswap_memcpy(tmp1, s1, size, elem_size);
    bswap_memcpy(tmp2, s2, size, elem_size);

    result = pmemcmp(tmp1, tmp2, size);

    pfree(tmp1);
    pfree(tmp2);

    return result;
}

extern void *pino_endianness_memcpy_le2native(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memcpy_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memcpy_be2native(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memcpy_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memcpy_native2le(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memcpy_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memcpy_native2be(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memcpy_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memmove_le2native(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memmove_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memmove_be2native(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memmove_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memmove_native2le(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memmove_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memmove_native2be(void *dest, const void *src, size_t size, size_t elem_size)
{
    return memmove_common(dest, src, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}

extern int pino_endianness_memcmp_le2native(const void *s1, const void *s2, size_t size, size_t elem_size)
{
    return memcmp_common(s1, s2, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern int pino_endianness_memcmp_be2native(const void *s1, const void *s2, size_t size, size_t elem_size)
{
    return memcmp_common(s1, s2, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}

extern int pino_endianness_memcmp_native2le(const void *s1, const void *s2, size_t size, size_t elem_size)
{
    return memcmp_common(s1, s2, size, elem_size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern int pino_endianness_memcmp_native2be(const void *s1, const void *s2, size_t size, size_t elem_size)
{
    return memcmp_common(s1, s2, size, elem_size, (platform_endianness() == ENDIANNESS_BIG));
}
