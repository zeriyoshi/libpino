/*
 * libpino - test_uaf.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <pino.h>
#include <pino/handler.h>

#include "../src/internal/common.h"
#include "handler_spl1.h"
#include "unity.h"
#include "util.h"

#define TEST_DATA_SIZE 256

void setUp(void)
{
    if (!pino_init()) {
        TEST_FAIL();
    }
}

void tearDown(void)
{
    pino_free();
}

void test_destroy_after_unregister(void)
{
    pino_t *pino;
    uint8_t data[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));

    pino_destroy(pino);
}

void test_unpack_after_unregister(void)
{
    pino_t *pino;
    uint8_t data[TEST_DATA_SIZE], unpacked[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));

    TEST_ASSERT_EQUAL_size_t(TEST_DATA_SIZE, pino_unpack_size(pino));
    TEST_ASSERT_TRUE(pino_unpack(pino, unpacked));
    TEST_ASSERT_EQUAL_MEMORY(data, unpacked, TEST_DATA_SIZE);

    pino_destroy(pino);
}

void test_multiple_pinos_after_unregister(void)
{
    pino_t *pinos[10];
    uint8_t data[TEST_DATA_SIZE], unpacked[TEST_DATA_SIZE];
    size_t i;

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));

    for (i = 0; i < 10; i++) {
        pinos[i] = pino_pack("spl1", data, TEST_DATA_SIZE);
        TEST_ASSERT_NOT_NULL(pinos[i]);
    }

    TEST_ASSERT_TRUE(PH_UNREG(spl1));

    for (i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL_size_t(TEST_DATA_SIZE, pino_unpack_size(pinos[i]));
        TEST_ASSERT_TRUE(pino_unpack(pinos[i], unpacked));
        TEST_ASSERT_EQUAL_MEMORY(data, unpacked, TEST_DATA_SIZE);
        pino_destroy(pinos[i]);
    }
}

void test_serialize_after_unregister(void)
{
    pino_t *pino;
    uint8_t data[TEST_DATA_SIZE], *serialized;
    size_t serialized_size;

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));

    serialized_size = pino_serialize_size(pino);
    TEST_ASSERT_GREATER_THAN_size_t(0, serialized_size);

    serialized = (uint8_t *)malloc(serialized_size);
    TEST_ASSERT_NOT_NULL(serialized);
    TEST_ASSERT_TRUE(pino_serialize(pino, serialized));

    pino_destroy(pino);
    free(serialized);
}

void test_unregister_no_live_objects(void)
{
    pino_t *pino;
    uint8_t data[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);
    pino_destroy(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));
}

void test_reregister_after_deferred_cleanup(void)
{
    pino_t *pino, *pino2;
    uint8_t data[TEST_DATA_SIZE], unpacked[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));
    pino_destroy(pino);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino2 = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino2);

    TEST_ASSERT_EQUAL_size_t(TEST_DATA_SIZE, pino_unpack_size(pino2));
    TEST_ASSERT_TRUE(pino_unpack(pino2, unpacked));
    TEST_ASSERT_EQUAL_MEMORY(data, unpacked, TEST_DATA_SIZE);

    pino_destroy(pino2);
    TEST_ASSERT_TRUE(PH_UNREG(spl1));
}

void test_pino_free_with_unregistered_live_objects(void)
{
    pino_t *pino;
    uint8_t data[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(PH_REG(spl1));
    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_TRUE(PH_UNREG(spl1));

    pino_destroy(pino);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_destroy_after_unregister);
    RUN_TEST(test_unpack_after_unregister);
    RUN_TEST(test_multiple_pinos_after_unregister);
    RUN_TEST(test_serialize_after_unregister);
    RUN_TEST(test_unregister_no_live_objects);
    RUN_TEST(test_reregister_after_deferred_cleanup);
    RUN_TEST(test_pino_free_with_unregistered_live_objects);

    return UNITY_END();
}
