/*
 * libpino - bswap.h
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef PINO_INTERNAL_BSWAP_H
#define PINO_INTERNAL_BSWAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <pino/portable.h>

#if PINO_USE_SIMD
#include "simd.h"
#endif

static inline void *pino_bswap_memcpy(void *dest, const void *src, size_t size, size_t elem_size)
{
    const uint8_t *sp;
    uint64_t *dest64, *src64;
    uint32_t *dest32, *src32;
    uint16_t *dest16, *src16;
    uint8_t *dp;
    size_t i, j, num_elements;

#if PINO_USE_SIMD && defined(PINO_SIMD_AVX2)
    __m256i shuffle_mask, data;
    size_t simd_elements;

    dp = (uint8_t *)dest;
    sp = (const uint8_t *)src;

    if (elem_size == 1 || size == 0) {
        return memcpy(dest, src, size);
    }

    if (size % elem_size != 0) {
        return memcpy(dest, src, size);
    }

    if (elem_size == 2 && (((uintptr_t)dest | (uintptr_t)src) & 0x1) == 0) {
        dest16 = (uint16_t *)dest;
        src16 = (uint16_t *)src;
        num_elements = size / 2;

        for (i = 0; i < num_elements; i++) {
            dest16[i] = pino_bswap16(src16[i]);
        }

        return dest;
    } else if (elem_size == 4 && (((uintptr_t)dest | (uintptr_t)src) & 0x3) == 0) {
        dest32 = (uint32_t *)dest;
        src32 = (uint32_t *)src;
        num_elements = size / 4;

        simd_elements = num_elements / 8;

        if (simd_elements > 0) {
            shuffle_mask = _mm256_set_epi8(28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19, 12, 13, 14,
                                           15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);

            for (i = 0; i < simd_elements; i++) {
                data = _mm256_loadu_si256((__m256i *)(src32 + i * 8));
                data = _mm256_shuffle_epi8(data, shuffle_mask);
                _mm256_storeu_si256((__m256i *)(dest32 + i * 8), data);
            }

            i = simd_elements * 8;
        } else {
            i = 0;
        }

        for (; i < num_elements; i++) {
            dest32[i] = pino_bswap32(src32[i]);
        }

        return dest;
    } else if (elem_size == 8 && (((uintptr_t)dest | (uintptr_t)src) & 0x7) == 0) {
        dest64 = (uint64_t *)dest;
        src64 = (uint64_t *)src;
        num_elements = size / 8;

        simd_elements = num_elements / 4;

        if (simd_elements > 0) {
            shuffle_mask = _mm256_set_epi8(24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23, 8, 9, 10, 11,
                                           12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7);

            for (i = 0; i < simd_elements; i++) {
                data = _mm256_loadu_si256((__m256i *)(src64 + i * 4));
                data = _mm256_shuffle_epi8(data, shuffle_mask);
                _mm256_storeu_si256((__m256i *)(dest64 + i * 4), data);
            }

            i = simd_elements * 4;
        } else {
            i = 0;
        }

        for (; i < num_elements; i++) {
            dest64[i] = pino_bswap64(src64[i]);
        }

        return dest;
    }

    num_elements = size / elem_size;
    for (i = 0; i < num_elements; i++) {
        for (j = 0; j < elem_size; j++) {
            dp[i * elem_size + j] = sp[i * elem_size + (elem_size - 1 - j)];
        }
    }

    return dest;

#elif PINO_USE_SIMD && defined(PINO_SIMD_NEON)
    uint16x8_t data16;
    uint32x4_t data32;
    uint64x2_t data64;
    size_t simd_elements;

    dp = (uint8_t *)dest;
    sp = (const uint8_t *)src;

    if (elem_size == 1 || size == 0) {
        return memcpy(dest, src, size);
    }

    if (size % elem_size != 0) {
        return memcpy(dest, src, size);
    }

    if (elem_size == 2 && (((uintptr_t)dest | (uintptr_t)src) & 0x1) == 0) {
        dest16 = (uint16_t *)dest;
        src16 = (uint16_t *)src;
        num_elements = size / 2;

        simd_elements = num_elements / 8;

        for (i = 0; i < simd_elements; i++) {
            data16 = vld1q_u16(src16 + i * 8);
            data16 = vreinterpretq_u16_u8(vrev16q_u8(vreinterpretq_u8_u16(data16)));
            vst1q_u16(dest16 + i * 8, data16);
        }

        i = simd_elements * 8;
        for (; i < num_elements; i++) {
            dest16[i] = pino_bswap16(src16[i]);
        }

        return dest;
    } else if (elem_size == 4 && (((uintptr_t)dest | (uintptr_t)src) & 0x3) == 0) {
        dest32 = (uint32_t *)dest;
        src32 = (uint32_t *)src;
        num_elements = size / 4;

        simd_elements = num_elements / 4;

        for (i = 0; i < simd_elements; i++) {
            data32 = vld1q_u32(src32 + i * 4);
            data32 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(data32)));
            vst1q_u32(dest32 + i * 4, data32);
        }

        i = simd_elements * 4;
        for (; i < num_elements; i++) {
            dest32[i] = pino_bswap32(src32[i]);
        }

        return dest;
    } else if (elem_size == 8 && (((uintptr_t)dest | (uintptr_t)src) & 0x7) == 0) {
        dest64 = (uint64_t *)dest;
        src64 = (uint64_t *)src;
        num_elements = size / 8;

        simd_elements = num_elements / 2;

        for (i = 0; i < simd_elements; i++) {
            data64 = vld1q_u64(src64 + i * 2);
            data64 = vreinterpretq_u64_u8(vrev64q_u8(vreinterpretq_u8_u64(data64)));
            vst1q_u64(dest64 + i * 2, data64);
        }

        i = simd_elements * 2;
        for (; i < num_elements; i++) {
            dest64[i] = pino_bswap64(src64[i]);
        }

        return dest;
    }

    num_elements = size / elem_size;
    for (i = 0; i < num_elements; i++) {
        for (j = 0; j < elem_size; j++) {
            dp[i * elem_size + j] = sp[i * elem_size + (elem_size - 1 - j)];
        }
    }

    return dest;

#elif PINO_USE_SIMD && defined(PINO_SIMD_WASM)
    v128_t data;
    size_t simd_elements;

    dp = (uint8_t *)dest;
    sp = (const uint8_t *)src;

    if (elem_size == 1 || size == 0) {
        return memcpy(dest, src, size);
    }

    if (size % elem_size != 0) {
        return memcpy(dest, src, size);
    }

    if (elem_size == 2 && (((uintptr_t)dest | (uintptr_t)src) & 0x1) == 0) {
        dest16 = (uint16_t *)dest;
        src16 = (uint16_t *)src;
        num_elements = size / 2;

        for (i = 0; i < num_elements; i++) {
            dest16[i] = pino_bswap16(src16[i]);
        }

        return dest;
    } else if (elem_size == 4 && (((uintptr_t)dest | (uintptr_t)src) & 0x3) == 0) {
        dest32 = (uint32_t *)dest;
        src32 = (uint32_t *)src;
        num_elements = size / 4;

        simd_elements = num_elements / 4;

        if (simd_elements > 0) {
            for (i = 0; i < simd_elements; i++) {
                data = wasm_v128_load(src32 + i * 4);
                data = wasm_i8x16_shuffle(data, data, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12);
                wasm_v128_store(dest32 + i * 4, data);
            }

            i = simd_elements * 4;
        } else {
            i = 0;
        }

        for (; i < num_elements; i++) {
            dest32[i] = pino_bswap32(src32[i]);
        }

        return dest;
    } else if (elem_size == 8 && (((uintptr_t)dest | (uintptr_t)src) & 0x7) == 0) {
        dest64 = (uint64_t *)dest;
        src64 = (uint64_t *)src;
        num_elements = size / 8;

        simd_elements = num_elements / 2;

        if (simd_elements > 0) {
            for (i = 0; i < simd_elements; i++) {
                data = wasm_v128_load(src64 + i * 2);
                data = wasm_i8x16_shuffle(data, data, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8);
                wasm_v128_store(dest64 + i * 2, data);
            }

            i = simd_elements * 2;
        } else {
            i = 0;
        }

        for (; i < num_elements; i++) {
            dest64[i] = pino_bswap64(src64[i]);
        }

        return dest;
    }

    num_elements = size / elem_size;
    for (i = 0; i < num_elements; i++) {
        for (j = 0; j < elem_size; j++) {
            dp[i * elem_size + j] = sp[i * elem_size + (elem_size - 1 - j)];
        }
    }

    return dest;

#else

    dp = (uint8_t *)dest;
    sp = (const uint8_t *)src;

    if (elem_size == 1 || size == 0) {
        return memcpy(dest, src, size);
    }

    if (size % elem_size != 0) {
        return memcpy(dest, src, size);
    }

    if (elem_size == 2 && (((uintptr_t)dest | (uintptr_t)src) & 0x1) == 0) {
        dest16 = (uint16_t *)dest;
        src16 = (uint16_t *)src;
        num_elements = size / 2;

        for (i = 0; i < num_elements; i++) {
            dest16[i] = pino_bswap16(src16[i]);
        }

        return dest;
    } else if (elem_size == 4 && (((uintptr_t)dest | (uintptr_t)src) & 0x3) == 0) {
        dest32 = (uint32_t *)dest;
        src32 = (uint32_t *)src;
        num_elements = size / 4;

        for (i = 0; i < num_elements; i++) {
            dest32[i] = pino_bswap32(src32[i]);
        }

        return dest;
    } else if (elem_size == 8 && (((uintptr_t)dest | (uintptr_t)src) & 0x7) == 0) {
        dest64 = (uint64_t *)dest;
        src64 = (uint64_t *)src;
        num_elements = size / 8;

        for (i = 0; i < num_elements; i++) {
            dest64[i] = pino_bswap64(src64[i]);
        }

        return dest;
    }

    num_elements = size / elem_size;
    for (i = 0; i < num_elements; i++) {
        for (j = 0; j < elem_size; j++) {
            dp[i * elem_size + j] = sp[i * elem_size + (elem_size - 1 - j)];
        }
    }

    return dest;
#endif
}

#endif /* PINO_INTERNAL_BSWAP_H */
