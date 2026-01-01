/*
 * libpino - endianness.c
 * 
 */

#include <pino/endianness.h>

#include <pino_internal.h>

#define ENDIANNESS_UNKNOWN      0
#define ENDIANNESS_LITTLE       1
#define ENDIANNESS_BIG          2

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static uint8_t g_endianness = ENDIANNESS_LITTLE;
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
static uint8_t g_endianness = ENDIANNESS_BIG;
#else
static uint8_t g_endianness = ENDIANNESS_UNKNOWN;
#endif

static inline uint8_t platform_endianness(void)
{
    uint32_t i;
    uint8_t *p;
    
    if (g_endianness != ENDIANNESS_UNKNOWN) {
        return g_endianness;
    }

    /* LCOV_EXCL_START */

    i = 1;
    p = (uint8_t *)&i;

    return g_endianness = (p[0] == 1) ? ENDIANNESS_LITTLE : ENDIANNESS_BIG;

    /* LCOV_EXCL_STOP */
}

static inline uint16_t bswap16(uint16_t x)
{
    return ((x & 0xff) << 8) | ((x & 0xff00) >> 8);
}

static inline uint32_t bswap32(uint32_t x)
{
    return ((x & 0xff) << 24) | 
           ((x & 0xff00) << 8) | 
           ((x & 0xff0000) >> 8) | 
           ((x & 0xff000000) >> 24);
}

static inline uint64_t bswap64(uint64_t x)
{
    return ((x & 0xff) << 56) | 
           ((x & 0xff00) << 40) | 
           ((x & 0xff0000) << 24) | 
           ((x & 0xff000000) << 8) | 
           ((x & 0xff00000000ULL) >> 8) | 
           ((x & 0xff0000000000ULL) >> 24) | 
           ((x & 0xff000000000000ULL) >> 40) | 
           ((x & 0xff00000000000000ULL) >> 56);
}

static inline void *bswap_memcpy(void *dest, const void *src, size_t size, size_t elem_size)
{
    uint8_t *dp;
    const uint8_t *sp;
    size_t i, j;
    uint16_t *dest16, *src16;
    uint32_t *dest32, *src32;
    uint64_t *dest64, *src64;

    dp = (uint8_t *)dest;
    sp = (const uint8_t *)src;

    if (elem_size == 1 || size == 0) {
        return pmemcpy(dest, src, size);
    }

    if (elem_size == 2 && (((uintptr_t)dest | (uintptr_t)src) & 0x1) == 0) {
        dest16 = (uint16_t *)dest;
        src16 = (uint16_t *)src;

        for (i = 0; i < size / 2; i++) {
            dest16[i] = bswap16(src16[i]);
        }

        return dest;
    } else if (elem_size == 4 && (((uintptr_t)dest | (uintptr_t)src) & 0x3) == 0) {
        dest32 = (uint32_t *)dest;
        src32 = (uint32_t *)src;

        for (i = 0; i < size / 4; i++) {
            dest32[i] = bswap32(src32[i]);
        }

        return dest;
    } else if (elem_size == 8 && (((uintptr_t)dest | (uintptr_t)src) & 0x7) == 0) {
        dest64 = (uint64_t *)dest;
        src64 = (uint64_t *)src;

        for (i = 0; i < size / 8; i++) {
            dest64[i] = bswap64(src64[i]);
        }

        return dest;
    }

    for (i = 0; i < size; i += elem_size) {
        for (j = 0; j < elem_size; j++) {
            dp[i + j] = sp[i + (elem_size - 1 - j)];
        }
    }

    return dest;
}

static inline size_t elem_sizeof(size_t size)
{
    if (size == 1 || size == 2 || size == 4 || size == 8) {
        return size;
    }

    return 1;
}

static inline void *conv_memcpy(void *dest, const void *src, size_t size)
{
    return bswap_memcpy(dest, src, size, elem_sizeof(size));
}

static inline void *memcpy_common(void *dest, const void *src, size_t size, bool is_native)
{
    return is_native ? pmemcpy(dest, src, size) : conv_memcpy(dest, src, size);
}

static inline void *memmove_common(void *dest, const void *src, size_t size, bool is_native)
{
    uint8_t *tmp;

    if (is_native) {
        return pmemmove(dest, src, size);
    } else {
        tmp = (uint8_t *)pmalloc(size);
        /* LCOV_EXCL_START */
        if (tmp == NULL) {
            PINO_SUPRTF("pmalloc failed");
            return NULL;
        }
        /* LCOV_EXCL_STOP */

        pmemcpy(tmp, src, size);
        bswap_memcpy(dest, tmp, size, elem_sizeof(size));

        pfree(tmp);

        return dest;
    }

    /* LCOV_EXCL_START */
    PINO_SUPUNREACH();
    return NULL;
    /* LCOV_EXCL_STOP */
}

static inline int memcmp_common(const void *s1, const void *s2, size_t size, bool is_native)
{
    uint8_t *tmp1, *tmp2;
    size_t elem_size;
    int result;

    if (is_native) {
        return pmemcmp(s1, s2, size);
    } else {
        tmp1 = (uint8_t *)pmalloc(size);
        tmp2 = (uint8_t *)pmalloc(size);
        /* LCOV_EXCL_START */
        if (!tmp1 || !tmp2) {
            PINO_SUPRTF("pmalloc failed");

            if (tmp1) {
                pfree(tmp1);
            }

            if (tmp2) {
                pfree(tmp2);
            }

            return 0;
        }
        /* LCOV_EXCL_STOP */

        elem_size = elem_sizeof(size);
        bswap_memcpy(tmp1, s1, size, elem_size);
        bswap_memcpy(tmp2, s2, size, elem_size);

        result = pmemcmp(tmp1, tmp2, size);

        pfree(tmp1);
        pfree(tmp2);

        return result;
    }

    /* LCOV_EXCL_START */
    PINO_SUPUNREACH();
    return 0;
    /* LCOV_EXCL_STOP */
}

extern void *pino_endianness_memcpy_le2native(void *dest, const void *src, size_t size)
{    
    return memcpy_common(dest, src, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memcpy_be2native(void *dest, const void *src, size_t size)
{   
    return memcpy_common(dest, src, size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memcpy_native2le(void *dest, const void *src, size_t size)
{
    return memcpy_common(dest, src, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memcpy_native2be(void *dest, const void *src, size_t size)
{
    return memcpy_common(dest, src, size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memmove_le2native(void *dest, const void *src, size_t size)
{
    return memmove_common(dest, src, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memmove_be2native(void *dest, const void *src, size_t size)
{
    return memmove_common(dest, src, size, (platform_endianness() == ENDIANNESS_BIG));
}

extern void *pino_endianness_memmove_native2le(void *dest, const void *src, size_t size)
{
    return memmove_common(dest, src, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern void *pino_endianness_memmove_native2be(void *dest, const void *src, size_t size)
{
    return memmove_common(dest, src, size, (platform_endianness() == ENDIANNESS_BIG));
}

extern int pino_endianness_memcmp_le2native(const void *s1, const void *s2, size_t size)
{
    return memcmp_common(s1, s2, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern int pino_endianness_memcmp_be2native(const void *s1, const void *s2, size_t size)
{
    return memcmp_common(s1, s2, size, (platform_endianness() == ENDIANNESS_BIG));
}

extern int pino_endianness_memcmp_native2le(const void *s1, const void *s2, size_t size)
{
    return memcmp_common(s1, s2, size, (platform_endianness() == ENDIANNESS_LITTLE));
}

extern int pino_endianness_memcmp_native2be(const void *s1, const void *s2, size_t size)
{
    return memcmp_common(s1, s2, size, (platform_endianness() == ENDIANNESS_BIG));
}
