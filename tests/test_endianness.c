/*
 * libpino test - test_endianness.c
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <pino/handler.h>
#include <pino/endianness.h>

#include "util.h"

#include "unity.h"

#if INTPTR_MAX > INT32_MAX
# define HAS_64BIT 1
#else
# define HAS_64BIT 0
#endif

typedef struct {
    uint16_t val16;
    uint32_t val32;
#if HAS_64BIT
    uint64_t val64;
#endif
} endianness_test_t;

static inline bool is_little_endian(void)
{
    uint32_t i = 1;
    uint8_t *p = (uint8_t *)&i;
    return p[0] == 1;
}

static inline void prepare_le_data(endianness_test_t *data)
{
    data->val16 = 0x1234;
    data->val32 = 0x12345678;
#if HAS_64BIT
    data->val64 = 0x1234567890ABCDEF;
#endif
}

static inline void prepare_be_data(endianness_test_t *data)
{
    data->val16 = 0x3412;
    data->val32 = 0x78563412;
#if HAS_64BIT
    data->val64 = 0xEFCDAB9078563412;
#endif
}

static inline void swap_16(uint16_t *val)
{
    uint8_t *p = (uint8_t *)val;
    uint8_t tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;
}

static inline void swap_32(uint32_t *val)
{
    uint8_t *p = (uint8_t *)val;
    uint8_t tmp;

    tmp = p[0]; p[0] = p[3]; p[3] = tmp;
    tmp = p[1]; p[1] = p[2]; p[2] = tmp;
}

#if HAS_64BIT
static inline void swap_64(uint64_t *val)
{
    uint8_t *p = (uint8_t *)val;
    uint8_t tmp;

    tmp = p[0]; p[0] = p[7]; p[7] = tmp;
    tmp = p[1]; p[1] = p[6]; p[6] = tmp;
    tmp = p[2]; p[2] = p[5]; p[5] = tmp;
    tmp = p[3]; p[3] = p[4]; p[4] = tmp;
}
#endif

void setUp(void)
{
}

void tearDown(void)
{
}

void test_memcpy_l2n(void)
{
    endianness_test_t src, dest, expected;

    prepare_le_data(&src);
    memset(&dest, 0, sizeof(dest));

    if (is_little_endian()) {
        expected = src;
    } else {
        expected = src;
        swap_16(&expected.val16);
        swap_32(&expected.val32);
#if HAS_64BIT
        swap_64(&expected.val64);
#endif
    }

    pino_endianness_memcpy_le2native(&dest.val16, &src.val16, sizeof(uint16_t));
    TEST_ASSERT_EQUAL_HEX16(expected.val16, dest.val16);

    pino_endianness_memcpy_le2native(&dest.val32, &src.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_HEX32(expected.val32, dest.val32);

#if HAS_64BIT
    pino_endianness_memcpy_le2native(&dest.val64, &src.val64, sizeof(uint64_t));
    TEST_ASSERT_EQUAL_HEX64(expected.val64, dest.val64);
#endif
}

void test_memcpy_b2n(void)
{
    endianness_test_t src, dest, expected;

    prepare_be_data(&src);
    memset(&dest, 0, sizeof(dest));

    if (is_little_endian()) {
        expected = src;
        swap_16(&expected.val16);
        swap_32(&expected.val32);
#if HAS_64BIT
        swap_64(&expected.val64);
#endif
    } else {
        expected = src;
    }

    pino_endianness_memcpy_be2native(&dest.val16, &src.val16, sizeof(uint16_t));
    TEST_ASSERT_EQUAL_HEX16(expected.val16, dest.val16);

    pino_endianness_memcpy_be2native(&dest.val32, &src.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_HEX32(expected.val32, dest.val32);

#if HAS_64BIT
    pino_endianness_memcpy_be2native(&dest.val64, &src.val64, sizeof(uint64_t));
    TEST_ASSERT_EQUAL_HEX64(expected.val64, dest.val64);
#endif
}

void test_memcpy_n2l(void)
{
    endianness_test_t src, dest, expected;

    if (is_little_endian()) {
        prepare_le_data(&src);
        expected = src;
    } else {
        prepare_be_data(&src);
        expected = src;
        swap_16(&expected.val16);
        swap_32(&expected.val32);
#if HAS_64BIT
        swap_64(&expected.val64);
#endif
    }

    memset(&dest, 0, sizeof(dest));

    pino_endianness_memcpy_native2le(&dest.val16, &src.val16, sizeof(uint16_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX16(src.val16, dest.val16);
    } else {
        uint16_t swapped = src.val16;
        swap_16(&swapped);
        TEST_ASSERT_EQUAL_HEX16(swapped, dest.val16);
    }

    pino_endianness_memcpy_native2le(&dest.val32, &src.val32, sizeof(uint32_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX32(src.val32, dest.val32);
    } else {
        uint32_t swapped = src.val32;
        swap_32(&swapped);
        TEST_ASSERT_EQUAL_HEX32(swapped, dest.val32);
    }

#if HAS_64BIT
    pino_endianness_memcpy_native2le(&dest.val64, &src.val64, sizeof(uint64_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX64(src.val64, dest.val64);
    } else {
        uint64_t swapped = src.val64;
        swap_64(&swapped);
        TEST_ASSERT_EQUAL_HEX64(swapped, dest.val64);
    }
#endif
}

void test_memcpy_n2b(void)
{
    endianness_test_t src, dest;

    if (is_little_endian()) {
        prepare_le_data(&src);
    } else {
        prepare_be_data(&src);
    }

    memset(&dest, 0, sizeof(dest));

    pino_endianness_memcpy_native2be(&dest.val16, &src.val16, sizeof(uint16_t));

    if (is_little_endian()) {
        uint16_t swapped = src.val16;
        swap_16(&swapped);
        TEST_ASSERT_EQUAL_HEX16(swapped, dest.val16);
    } else {
        TEST_ASSERT_EQUAL_HEX16(src.val16, dest.val16);
    }

    pino_endianness_memcpy_native2be(&dest.val32, &src.val32, sizeof(uint32_t));

    if (is_little_endian()) {
        uint32_t swapped = src.val32;
        swap_32(&swapped);
        TEST_ASSERT_EQUAL_HEX32(swapped, dest.val32);
    } else {
        TEST_ASSERT_EQUAL_HEX32(src.val32, dest.val32);
    }

#if HAS_64BIT
    pino_endianness_memcpy_native2be(&dest.val64, &src.val64, sizeof(uint64_t));

    if (is_little_endian()) {
        uint64_t swapped = src.val64;
        swap_64(&swapped);
        TEST_ASSERT_EQUAL_HEX64(swapped, dest.val64);
    } else {
        TEST_ASSERT_EQUAL_HEX64(src.val64, dest.val64);
    }
#endif
}

void test_memmove_l2n(void)
{
    endianness_test_t src, dest, expected;

    prepare_le_data(&src);
    memset(&dest, 0, sizeof(dest));

    if (is_little_endian()) {
        expected = src;
    } else {
        expected = src;
        swap_16(&expected.val16);
        swap_32(&expected.val32);
#if HAS_64BIT
        swap_64(&expected.val64);
#endif
    }

    pino_endianness_memmove_le2native(&dest.val16, &src.val16, sizeof(uint16_t));
    TEST_ASSERT_EQUAL_HEX16(expected.val16, dest.val16);

    pino_endianness_memmove_le2native(&dest.val32, &src.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_HEX32(expected.val32, dest.val32);

#if HAS_64BIT
    pino_endianness_memmove_le2native(&dest.val64, &src.val64, sizeof(uint64_t));
    TEST_ASSERT_EQUAL_HEX64(expected.val64, dest.val64);

    uint8_t buffer[8] = {0};
    uint8_t expected_bytes[8];

    if (is_little_endian()) {
        buffer[0] = 0xEF;
        buffer[1] = 0xCD;
        buffer[2] = 0xAB;
        buffer[3] = 0x90;
        buffer[4] = 0x78;
        buffer[5] = 0x56;
        buffer[6] = 0x34;
        buffer[7] = 0x12;

        memcpy(expected_bytes, buffer, 8);
    } else {
        buffer[0] = 0xEF;
        buffer[1] = 0xCD;
        buffer[2] = 0xAB;
        buffer[3] = 0x90;
        buffer[4] = 0x78;
        buffer[5] = 0x56;
        buffer[6] = 0x34;
        buffer[7] = 0x12;

        expected_bytes[0] = 0x12;
        expected_bytes[1] = 0x34;
        expected_bytes[2] = 0x56;
        expected_bytes[3] = 0x78;
        expected_bytes[4] = 0x90;
        expected_bytes[5] = 0xAB;
        expected_bytes[6] = 0xCD;
        expected_bytes[7] = 0xEF;
    }

    pino_endianness_memmove_le2native(buffer, buffer, sizeof(uint64_t));

    TEST_ASSERT_EQUAL_MEMORY(expected_bytes, buffer, sizeof(uint64_t));
#endif
}

void test_memmove_b2n(void)
{
    endianness_test_t src, dest, expected;

    prepare_be_data(&src);
    memset(&dest, 0, sizeof(dest));

    if (is_little_endian()) {
        expected = src;
        swap_16(&expected.val16);
        swap_32(&expected.val32);
#if HAS_64BIT
        swap_64(&expected.val64);
#endif
    } else {
        expected = src;
    }

    pino_endianness_memmove_be2native(&dest.val16, &src.val16, sizeof(uint16_t));
    TEST_ASSERT_EQUAL_HEX16(expected.val16, dest.val16);

    pino_endianness_memmove_be2native(&dest.val32, &src.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_HEX32(expected.val32, dest.val32);

#if HAS_64BIT
    pino_endianness_memmove_be2native(&dest.val64, &src.val64, sizeof(uint64_t));
    TEST_ASSERT_EQUAL_HEX64(expected.val64, dest.val64);

    uint8_t buffer[16] = {0};
    uint64_t expected_val;

    if (is_little_endian()) {
        uint64_t le_val = 0x1234567890ABCDEF;
        memcpy(buffer, &le_val, sizeof(uint64_t));
        expected_val = 0xEFCDAB9078563412;
    } else {
        uint64_t be_val = 0xEFCDAB9078563412;
        memcpy(buffer, &be_val, sizeof(uint64_t));
        expected_val = 0xEFCDAB9078563412;
    }

    pino_endianness_memmove_native2be(buffer, buffer, sizeof(uint64_t));

    TEST_ASSERT_EQUAL_MEMORY(&expected_val, buffer, sizeof(uint64_t));
#endif
}

void test_memmove_n2l(void)
{
    endianness_test_t src, dest;

    if (is_little_endian()) {
        prepare_le_data(&src);
    } else {
        prepare_be_data(&src);
    }

    memset(&dest, 0, sizeof(dest));

    pino_endianness_memmove_native2le(&dest.val16, &src.val16, sizeof(uint16_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX16(src.val16, dest.val16);
    } else {
        uint16_t swapped = src.val16;
        swap_16(&swapped);
        TEST_ASSERT_EQUAL_HEX16(swapped, dest.val16);
    }

    pino_endianness_memmove_native2le(&dest.val32, &src.val32, sizeof(uint32_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX32(src.val32, dest.val32);
    } else {
        uint32_t swapped = src.val32;
        swap_32(&swapped);
        TEST_ASSERT_EQUAL_HEX32(swapped, dest.val32);
    }

#if HAS_64BIT
    pino_endianness_memmove_native2le(&dest.val64, &src.val64, sizeof(uint64_t));

    if (is_little_endian()) {
        TEST_ASSERT_EQUAL_HEX64(src.val64, dest.val64);
    } else {
        uint64_t swapped = src.val64;
        swap_64(&swapped);
        TEST_ASSERT_EQUAL_HEX64(swapped, dest.val64);
    }

    uint8_t buffer[8] = {0};
    uint8_t expected_bytes[8];

    if (is_little_endian()) {
        buffer[0] = 0xEF;
        buffer[1] = 0xCD;
        buffer[2] = 0xAB;
        buffer[3] = 0x90;
        buffer[4] = 0x78;
        buffer[5] = 0x56;
        buffer[6] = 0x34;
        buffer[7] = 0x12;

        memcpy(expected_bytes, buffer, 8);
    } else {
        buffer[0] = 0x12;
        buffer[1] = 0x34;
        buffer[2] = 0x56;
        buffer[3] = 0x78;
        buffer[4] = 0x90;
        buffer[5] = 0xAB;
        buffer[6] = 0xCD;
        buffer[7] = 0xEF;

        expected_bytes[0] = 0xEF;
        expected_bytes[1] = 0xCD;
        expected_bytes[2] = 0xAB;
        expected_bytes[3] = 0x90;
        expected_bytes[4] = 0x78;
        expected_bytes[5] = 0x56;
        expected_bytes[6] = 0x34;
        expected_bytes[7] = 0x12;
    }

    pino_endianness_memmove_native2le(buffer, buffer, sizeof(uint64_t));

    TEST_ASSERT_EQUAL_MEMORY(expected_bytes, buffer, sizeof(uint64_t));
#endif
}

void test_memmove_n2b(void)
{
    endianness_test_t src, dest;

    if (is_little_endian()) {
        prepare_le_data(&src);
    } else {
        prepare_be_data(&src);
    }

    memset(&dest, 0, sizeof(dest));

    pino_endianness_memmove_native2be(&dest.val16, &src.val16, sizeof(uint16_t));

    if (is_little_endian()) {
        uint16_t swapped = src.val16;
        swap_16(&swapped);
        TEST_ASSERT_EQUAL_HEX16(swapped, dest.val16);
    } else {
        TEST_ASSERT_EQUAL_HEX16(src.val16, dest.val16);
    }

    pino_endianness_memmove_native2be(&dest.val32, &src.val32, sizeof(uint32_t));

    if (is_little_endian()) {
        uint32_t swapped = src.val32;
        swap_32(&swapped);
        TEST_ASSERT_EQUAL_HEX32(swapped, dest.val32);
    } else {
        TEST_ASSERT_EQUAL_HEX32(src.val32, dest.val32);
    }

#if HAS_64BIT
    pino_endianness_memmove_native2be(&dest.val64, &src.val64, sizeof(uint64_t));

    if (is_little_endian()) {
        uint64_t swapped = src.val64;
        swap_64(&swapped);
        TEST_ASSERT_EQUAL_HEX64(swapped, dest.val64);
    } else {
        TEST_ASSERT_EQUAL_HEX64(src.val64, dest.val64);
    }

    uint8_t buffer[16] = {0};
    uint64_t expected_val;

    if (is_little_endian()) {
        uint64_t le_val = 0x1234567890ABCDEF;
        memcpy(buffer, &le_val, sizeof(uint64_t));
        expected_val = 0xEFCDAB9078563412;
    } else {
        uint64_t be_val = 0xEFCDAB9078563412;
        memcpy(buffer, &be_val, sizeof(uint64_t));
        expected_val = 0xEFCDAB9078563412;
    }

    pino_endianness_memmove_native2be(buffer, buffer, sizeof(uint64_t));

    TEST_ASSERT_EQUAL_MEMORY(&expected_val, buffer, sizeof(uint64_t));
#endif
}

void test_memcmp_l2n(void)
{
    endianness_test_t data1, data2;
    int result;

    prepare_le_data(&data1);
    data2 = data1;

    result = pino_endianness_memcmp_le2native(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_INT(0, result);

    if (is_little_endian()) {
        data2.val32++;
        result = pino_endianness_memcmp_le2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result < 0);
    } else {
        uint32_t val = data1.val32;
        swap_32(&val);
        val++;
        swap_32(&val);
        data2.val32 = val;
        result = pino_endianness_memcmp_le2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result < 0);
    }

    if (is_little_endian()) {
        data2.val32 -= 2;
        result = pino_endianness_memcmp_le2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result > 0);
    } else {
        uint32_t val = data1.val32;
        swap_32(&val);
        val -= 2;
        swap_32(&val);
        data2.val32 = val;
        result = pino_endianness_memcmp_le2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result > 0);
    }
}

void test_memcmp_n2l(void)
{
    endianness_test_t data1, data2;
    int result;

    if (is_little_endian()) {
        prepare_le_data(&data1);
    } else {
        prepare_be_data(&data1);
    }
    data2 = data1;

    result = pino_endianness_memcmp_native2le(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_INT(0, result);

    data2.val32++;
    result = pino_endianness_memcmp_native2le(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_TRUE(result < 0);

    data2.val32 -= 2;
    result = pino_endianness_memcmp_native2le(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_TRUE(result > 0);
}

void test_memcmp_b2n(void)
{
    endianness_test_t data1, data2;
    int result;

    prepare_be_data(&data1);
    data2 = data1;

    result = pino_endianness_memcmp_be2native(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_INT(0, result);

    if (!is_little_endian()) {
        data2.val32++;
        result = pino_endianness_memcmp_be2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result < 0);
    } else {
        uint32_t val = data1.val32;
        swap_32(&val);
        val++;
        swap_32(&val);
        data2.val32 = val;
        result = pino_endianness_memcmp_be2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result < 0);
    }

    if (!is_little_endian()) {
        data2.val32 -= 2;
        result = pino_endianness_memcmp_be2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result > 0);
    } else {
        uint32_t val = data1.val32;
        swap_32(&val);
        val -= 2;
        swap_32(&val);
        data2.val32 = val;
        result = pino_endianness_memcmp_be2native(&data1.val32, &data2.val32, sizeof(uint32_t));
        TEST_ASSERT_TRUE(result > 0);
    }
}

void test_memcmp_n2b(void)
{
    endianness_test_t data1, data2;
    int result;

    if (is_little_endian()) {
        prepare_le_data(&data1);
    } else {
        prepare_be_data(&data1);
    }
    data2 = data1;

    result = pino_endianness_memcmp_native2be(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_INT(0, result);

    data2.val32++;
    result = pino_endianness_memcmp_native2be(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_TRUE(result < 0);

    data2.val32 -= 2;
    result = pino_endianness_memcmp_native2be(&data1.val32, &data2.val32, sizeof(uint32_t));
    TEST_ASSERT_TRUE(result > 0);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_memcpy_l2n);
    RUN_TEST(test_memcpy_b2n);
    RUN_TEST(test_memcpy_n2l);
    RUN_TEST(test_memcpy_n2b);

    RUN_TEST(test_memmove_l2n);
    RUN_TEST(test_memmove_b2n);
    RUN_TEST(test_memmove_n2l);
    RUN_TEST(test_memmove_n2b);

    RUN_TEST(test_memcmp_l2n);
    RUN_TEST(test_memcmp_b2n);
    RUN_TEST(test_memcmp_n2l);
    RUN_TEST(test_memcmp_n2b);

    return UNITY_END();
}
