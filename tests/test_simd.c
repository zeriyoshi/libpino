/*
 * libpino - test_simd.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/internal/bswap.h"
#include "unity.h"
#include "util.h"

#if INTPTR_MAX > INT32_MAX
#define HAS_64BIT 1
#else
#define HAS_64BIT 0
#endif

static inline bool is_little_endian(void)
{
    uint32_t i = 1;
    uint8_t *p = (uint8_t *)&i;

    return p[0] == 1;
}

static inline void swap_16(uint16_t *val)
{
    *val = pino_bswap16(*val);
}

static inline void swap_32(uint32_t *val)
{
    *val = pino_bswap32(*val);
}

#if HAS_64BIT
static inline void swap_64(uint64_t *val)
{
    *val = pino_bswap64(*val);
}
#endif

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bswap_32bit_simd_path(void)
{
    const size_t num_elements = 32;
    char msg[128];
    uint32_t src[32], dest[32], expected[32];
    size_t i;

    for (i = 0; i < num_elements; i++) {
        src[i] = 0x12345678 + (uint32_t)(i << 24);
    }

    memset(dest, 0, sizeof(dest));

    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint32_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_32(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "32-bit SIMD: Element %zu: expected 0x%08X, got 0x%08X", i, expected[i],
                     dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

void test_bswap_32bit_boundary(void)
{
    uint32_t src8[8], dest8[8], src7[7], dest7[7], src9[9], dest9[9], expected8[8], expected7[7], expected9[9];
    size_t i;
    char msg[128];

    for (i = 0; i < 8; i++) {
        src8[i] = 0xAABBCCDD + (uint32_t)(i << 24);
    }

    memset(dest8, 0, sizeof(dest8));
    pino_bswap_memcpy(dest8, src8, sizeof(src8), sizeof(uint32_t));

    for (i = 0; i < 8; i++) {
        expected8[i] = src8[i];
        swap_32(&expected8[i]);
    }

    for (i = 0; i < 8; i++) {
        if (dest8[i] != expected8[i]) {
            snprintf(msg, sizeof(msg), "8-elem boundary: Element %zu: expected 0x%08X, got 0x%08X", i, expected8[i],
                     dest8[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }

    for (i = 0; i < 7; i++) {
        src7[i] = 0x11223344 + (uint32_t)(i << 24);
    }

    memset(dest7, 0, sizeof(dest7));
    pino_bswap_memcpy(dest7, src7, sizeof(src7), sizeof(uint32_t));

    for (i = 0; i < 7; i++) {
        expected7[i] = src7[i];
        swap_32(&expected7[i]);
    }

    for (i = 0; i < 7; i++) {
        if (dest7[i] != expected7[i]) {
            snprintf(msg, sizeof(msg), "7-elem (scalar): Element %zu: expected 0x%08X, got 0x%08X", i, expected7[i],
                     dest7[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }

    for (i = 0; i < 9; i++) {
        src9[i] = 0x55667788 + (uint32_t)(i << 24);
    }

    memset(dest9, 0, sizeof(dest9));
    pino_bswap_memcpy(dest9, src9, sizeof(src9), sizeof(uint32_t));

    for (i = 0; i < 9; i++) {
        expected9[i] = src9[i];
        swap_32(&expected9[i]);
    }

    for (i = 0; i < 9; i++) {
        if (dest9[i] != expected9[i]) {
            snprintf(msg, sizeof(msg), "9-elem (mixed): Element %zu: expected 0x%08X, got 0x%08X", i, expected9[i],
                     dest9[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

#if HAS_64BIT
void test_bswap_64bit_simd_path(void)
{
    const size_t num_elements = 16;
    uint64_t src[16], dest[16], expected[16];
    size_t i;
    char msg[128];

    for (i = 0; i < num_elements; i++) {
        src[i] = 0x0123456789ABCDEFULL + ((uint64_t)i << 56);
    }

    memset(dest, 0, sizeof(dest));

    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint64_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_64(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "64-bit SIMD: Element %zu: expected 0x%016llX, got 0x%016llX", i,
                     (unsigned long long)expected[i], (unsigned long long)dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

void test_bswap_64bit_boundary(void)
{
    uint64_t src4[4], dest4[4], src3[3], dest3[3], src5[5], dest5[5], expected4[4], expected3[3], expected5[5];
    size_t i;
    char msg[128];

    for (i = 0; i < 4; i++) {
        src4[i] = 0x0102030405060708ULL + ((uint64_t)i << 56);
    }

    memset(dest4, 0, sizeof(dest4));
    pino_bswap_memcpy(dest4, src4, sizeof(src4), sizeof(uint64_t));

    for (i = 0; i < 4; i++) {
        expected4[i] = src4[i];
        swap_64(&expected4[i]);
    }

    for (i = 0; i < 4; i++) {
        if (dest4[i] != expected4[i]) {
            snprintf(msg, sizeof(msg),
                     "4-elem boundary: Element %zu: expected 0x%016llX, got "
                     "0x%016llX",
                     i, (unsigned long long)expected4[i], (unsigned long long)dest4[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }

    for (i = 0; i < 3; i++) {
        src3[i] = 0xFEDCBA9876543210ULL + ((uint64_t)i << 56);
    }

    memset(dest3, 0, sizeof(dest3));
    pino_bswap_memcpy(dest3, src3, sizeof(src3), sizeof(uint64_t));

    for (i = 0; i < 3; i++) {
        expected3[i] = src3[i];
        swap_64(&expected3[i]);
    }

    for (i = 0; i < 3; i++) {
        if (dest3[i] != expected3[i]) {
            snprintf(msg, sizeof(msg),
                     "3-elem (scalar): Element %zu: expected 0x%016llX, got "
                     "0x%016llX",
                     i, (unsigned long long)expected3[i], (unsigned long long)dest3[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }

    for (i = 0; i < 5; i++) {
        src5[i] = 0xA1B2C3D4E5F60718ULL + ((uint64_t)i << 56);
    }

    memset(dest5, 0, sizeof(dest5));
    pino_bswap_memcpy(dest5, src5, sizeof(src5), sizeof(uint64_t));

    for (i = 0; i < 5; i++) {
        expected5[i] = src5[i];
        swap_64(&expected5[i]);
    }

    for (i = 0; i < 5; i++) {
        if (dest5[i] != expected5[i]) {
            snprintf(msg, sizeof(msg),
                     "5-elem (mixed): Element %zu: expected 0x%016llX, got "
                     "0x%016llX",
                     i, (unsigned long long)expected5[i], (unsigned long long)dest5[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}
#endif

void test_bswap_16bit(void)
{
    const size_t num_elements = 16;
    uint16_t src[16], dest[16], expected[16];
    size_t i;
    char msg[128];

    for (i = 0; i < num_elements; i++) {
        src[i] = 0x1234 + (uint16_t)(i << 8);
    }

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint16_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_16(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "16-bit: Element %zu: expected 0x%04X, got 0x%04X", i, expected[i], dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

void test_bswap_byte_pattern(void)
{
    const size_t num_elements = 16;
    uint32_t src[16], dest[16], expected[16];
    size_t i;
    char msg[256];

    for (i = 0; i < num_elements; i++) {
        src[i] = 0x12345678 + (uint32_t)(i << 24);
    }

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint32_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_32(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "Element %zu: expected 0x%08X, got 0x%08X", i, expected[i], dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

void test_bswap_32bit_multi_iteration(void)
{
    const size_t num_elements = 64;
    uint32_t src[64], dest[64], expected[64];
    size_t i;
    char msg[128];

    for (i = 0; i < num_elements; i++) {
        src[i] = 0xDEADBEEF ^ (uint32_t)(i * 0x01010101);
    }

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint32_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_32(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "64-elem 32-bit: Element %zu: expected 0x%08X, got 0x%08X", i, expected[i],
                     dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

#if HAS_64BIT
void test_bswap_64bit_multi_iteration(void)
{
    const size_t num_elements = 32;
    uint64_t src[32], dest[32], expected[32];
    size_t i;
    char msg[128];

    for (i = 0; i < num_elements; i++) {
        src[i] = 0xDEADBEEFCAFEBABEULL ^ ((uint64_t)i * 0x0101010101010101ULL);
    }

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint64_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_64(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "32-elem 64-bit: Element %zu: expected 0x%016llX, got 0x%016llX", i,
                     (unsigned long long)expected[i], (unsigned long long)dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}
#endif

void test_bswap_standard_arrays(void)
{
    uint64_t src64[8], dest64[8], expected64[8];
    uint32_t src32[16], dest32[16], expected32[16];
    size_t i;
    char msg[128];

    for (i = 0; i < 16; i++) {
        src32[i] = 0x11223344 + (uint32_t)(i << 8);
    }

    memset(dest32, 0, sizeof(dest32));
    pino_bswap_memcpy(dest32, src32, sizeof(src32), sizeof(uint32_t));

    for (i = 0; i < 16; i++) {
        expected32[i] = src32[i];
        swap_32(&expected32[i]);
    }

    for (i = 0; i < 16; i++) {
        if (dest32[i] != expected32[i]) {
            snprintf(msg, sizeof(msg), "32-bit: Element %zu: expected 0x%08X, got 0x%08X", i, expected32[i], dest32[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }

#if HAS_64BIT
    for (i = 0; i < 8; i++) {
        src64[i] = 0x1122334455667788ULL + ((uint64_t)i << 8);
    }

    memset(dest64, 0, sizeof(dest64));
    pino_bswap_memcpy(dest64, src64, sizeof(src64), sizeof(uint64_t));

    for (i = 0; i < 8; i++) {
        expected64[i] = src64[i];
        swap_64(&expected64[i]);
    }

    for (i = 0; i < 8; i++) {
        if (dest64[i] != expected64[i]) {
            snprintf(msg, sizeof(msg),
                     "64-bit: Element %zu: expected 0x%016llX, got "
                     "0x%016llX",
                     i, (unsigned long long)expected64[i], (unsigned long long)dest64[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
#endif
}

void test_bswap_elem_size_1(void)
{
    uint8_t src[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, dest[16];

    memset(dest, 0xFF, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), 1);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(src, dest, 16);
}

void test_bswap_unaligned(void)
{
    uint32_t *src_unaligned, *dest_unaligned, expected[8];
    uint8_t buffer_src[40], buffer_dest[40];
    size_t i;

    src_unaligned = (uint32_t *)(buffer_src + 1);
    dest_unaligned = (uint32_t *)(buffer_dest + 1);

    for (i = 0; i < 8; i++) {
        src_unaligned[i] = 0x12345678 + (uint32_t)(i << 24);
    }

    memset(buffer_dest, 0, sizeof(buffer_dest));
    pino_bswap_memcpy(dest_unaligned, src_unaligned, 8 * sizeof(uint32_t), sizeof(uint32_t));

    for (i = 0; i < 8; i++) {
        expected[i] = src_unaligned[i];
        swap_32(&expected[i]);
    }

    for (i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL_HEX32(expected[i], dest_unaligned[i]);
    }
}

void test_bswap_16bit_large(void)
{
    const size_t num_elements = 64;
    uint16_t src[64], dest[64], expected[64];
    size_t i;
    char msg[128];

    for (i = 0; i < num_elements; i++) {
        src[i] = (uint16_t)(0x1234 + i * 0x0101);
    }

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, sizeof(src), sizeof(uint16_t));

    for (i = 0; i < num_elements; i++) {
        expected[i] = src[i];
        swap_16(&expected[i]);
    }

    for (i = 0; i < num_elements; i++) {
        if (dest[i] != expected[i]) {
            snprintf(msg, sizeof(msg), "16-bit large: Element %zu: expected 0x%04X, got 0x%04X", i, expected[i],
                     dest[i]);
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

void test_bswap_invalid_size(void)
{
    uint8_t src[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, dest[10];

    memset(dest, 0, sizeof(dest));
    pino_bswap_memcpy(dest, src, 10, 4);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(src, dest, 10);
}

void test_bswap_zero_size(void)
{
    uint32_t src[4] = {0x12345678, 0x9ABCDEF0, 0x11223344, 0x55667788}, dest[4] = {0, 0, 0, 0};

    pino_bswap_memcpy(dest, src, 0, sizeof(uint32_t));

    TEST_ASSERT_EQUAL_HEX32(0, dest[0]);
    TEST_ASSERT_EQUAL_HEX32(0, dest[1]);
    TEST_ASSERT_EQUAL_HEX32(0, dest[2]);
    TEST_ASSERT_EQUAL_HEX32(0, dest[3]);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_bswap_32bit_simd_path);
    RUN_TEST(test_bswap_32bit_boundary);
    RUN_TEST(test_bswap_32bit_multi_iteration);

#if HAS_64BIT
    RUN_TEST(test_bswap_64bit_simd_path);
    RUN_TEST(test_bswap_64bit_boundary);
    RUN_TEST(test_bswap_64bit_multi_iteration);
#endif

    RUN_TEST(test_bswap_16bit);
    RUN_TEST(test_bswap_byte_pattern);
    RUN_TEST(test_bswap_standard_arrays);
    RUN_TEST(test_bswap_elem_size_1);
    RUN_TEST(test_bswap_unaligned);

    RUN_TEST(test_bswap_16bit_large);
    RUN_TEST(test_bswap_invalid_size);
    RUN_TEST(test_bswap_zero_size);

    return UNITY_END();
}
