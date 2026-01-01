/*
 * libpino test - test_invalid.c
 * 
 */

#include <pino.h>
#include <pino/handler.h>
#include <pino/endianness.h>

#include "handler_spl1.h"

#include "util.h"

#include "unity.h"

#define TEST_DATA_SIZE 1024

static char g_invalid_static_fields_size_path[PATH_MAX];
static char g_truncated_path[PATH_MAX];
static char g_broken_path[PATH_MAX];
static char g_handler_missing_path[PATH_MAX];

void setUp(void)
{
    if (!pino_init() || !PH_REG(spl1)) {
        TEST_FAIL();
    }

    if (!get_asset_path("pack_invalid_static_fields_size.bin", g_invalid_static_fields_size_path, sizeof(g_invalid_static_fields_size_path)) ||
        !get_asset_path("pack_truncated.bin", g_truncated_path, sizeof(g_truncated_path)) ||
        !get_asset_path("pack_broken.bin", g_broken_path, sizeof(g_broken_path)) ||
        !get_asset_path("pack_handler_missing.bin", g_handler_missing_path, sizeof(g_handler_missing_path))
    ) {
        TEST_FAIL_MESSAGE("Failed to get asset path");
    }
}

void tearDown(void)
{
    if (!PH_UNREG(spl1)) {
        TEST_FAIL();
    }

    pino_free();
}

void test_pino(void)
{
    TEST_ASSERT_EQUAL_size_t(0, pino_serialize_size(NULL));
    TEST_ASSERT_FALSE(pino_serialize(NULL, NULL));
    TEST_ASSERT_NULL(pino_unserialize(NULL, 0));
    /* todo: pino_unserialize: invalid magic */
    /* todo: pino_unserialize: unregistered magic */
    TEST_ASSERT_NULL(pino_pack("abc\0", NULL, 0));
    /* todo: pino_pack: pino_create fail */
    /* todo: pino_pack: handler->pack fail */
    pino_destroy(NULL);
}

void test_endianness(void)
{
    char i = 1, j = 0, b[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, u[10] = {0};

    pino_endianness_memcpy_le2native(&i, &j, sizeof(char));
    pino_endianness_memcpy_be2native(&i, &j, sizeof(char));
    pino_endianness_memcpy_native2le(&i, &j, sizeof(char));
    pino_endianness_memcpy_native2be(&i, &j, sizeof(char));

    pino_endianness_memcpy_le2native(u + 1, b, 2);
    pino_endianness_memcpy_be2native(u + 1, b, 2);
    pino_endianness_memcpy_native2le(u + 1, b, 2);
    pino_endianness_memcpy_native2be(u + 1, b, 2);

    TEST_ASSERT_EQUAL_CHAR(0, i);
    TEST_ASSERT_EQUAL_CHAR(0, j);
}

void test_handler(void)
{
    TEST_ASSERT_FALSE(PH_REG(spl1)); /* already registered */
    TEST_ASSERT_FALSE(pino_handler_unregister("a\0\0\0"));
}

void test_invalid_static_fields_size(void)
{
    pino_t *pino;
    uint8_t *data;
    size_t size;

    TEST_ASSERT_TRUE(load_file(g_invalid_static_fields_size_path, &data, &size));
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_GREATER_THAN_size_t(0, size);

    pino = pino_unserialize(data, size);
    TEST_ASSERT_NULL(pino);

    free(data);
}

void test_truncated(void)
{
    pino_t *pino;
    uint8_t *data;
    size_t size;

    TEST_ASSERT_TRUE(load_file(g_truncated_path, &data, &size));
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_GREATER_THAN_size_t(0, size);

    pino = pino_unserialize(data, size);
    TEST_ASSERT_NULL(pino);

    free(data);
}

void test_broken(void)
{
    pino_t *pino;
    uint8_t *data;
    size_t size;

    TEST_ASSERT_TRUE(load_file(g_broken_path, &data, &size));
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_GREATER_THAN_size_t(0, size);

    pino = pino_unserialize(data, size);
    TEST_ASSERT_NULL(pino);

    free(data);
}

void test_handler_missing(void)
{
    pino_t *pino;
    uint8_t *data;
    size_t size;

    TEST_ASSERT_TRUE(load_file(g_handler_missing_path, &data, &size));
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_GREATER_THAN_size_t(0, size);

    pino = pino_unserialize(data, size);
    TEST_ASSERT_NULL(pino);

    free(data);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_pino);
    RUN_TEST(test_endianness);
    RUN_TEST(test_handler);

    RUN_TEST(test_invalid_static_fields_size);
    RUN_TEST(test_truncated);
    RUN_TEST(test_broken);
    RUN_TEST(test_handler_missing);

    return UNITY_END();
}
