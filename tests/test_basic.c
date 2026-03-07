/*
 * libpino - test_basic.c
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
#include "handler_u32a.h"
#include "unity.h"
#include "util.h"

#define TEST_DATA_SIZE 1024

static char g_asset_path[PATH_MAX];

void setUp(void)
{
    if (!pino_init() || !PH_REG(spl1)) {
        TEST_FAIL();
    }

    if (!get_asset_path("pack.bin", g_asset_path, sizeof(g_asset_path))) {
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

void test_register(void)
{
    TEST_ASSERT_TRUE(pino_handler_register("spl2", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_register("spl3", &g_ph_handler_spl1_obj));

    TEST_ASSERT_TRUE(pino_handler_unregister("spl2"));
    TEST_ASSERT_TRUE(pino_handler_register("spl2", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_unregister("spl2"));

    TEST_ASSERT_TRUE(pino_handler_register("spl4", &g_ph_handler_spl1_obj));

    TEST_ASSERT_TRUE(pino_handler_unregister("spl3"));
    TEST_ASSERT_TRUE(pino_handler_unregister("spl4"));
}

void test_register_fail(void)
{
    TEST_ASSERT_FALSE(pino_handler_register("spl1", &g_ph_handler_spl1_obj));
    TEST_ASSERT_FALSE(pino_handler_register("spl2", NULL));
    TEST_ASSERT_FALSE(pino_handler_register(NULL, &g_ph_handler_spl1_obj));
    TEST_ASSERT_FALSE(pino_handler_register("sapporo", &g_ph_handler_spl1_obj));
    TEST_ASSERT_FALSE(pino_handler_register("tky\0", &g_ph_handler_spl1_obj));
    TEST_ASSERT_FALSE(pino_handler_register("\0abc", &g_ph_handler_spl1_obj));
    TEST_ASSERT_FALSE(pino_handler_register("a\0b\0", &g_ph_handler_spl1_obj));
}

void test_register_unregistered(void)
{
    TEST_ASSERT_TRUE(pino_handler_register("spl2", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_register("spl3", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_register("spl4", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_register("spl5", &g_ph_handler_spl1_obj));
}

void test_register_glowing(void)
{
    pino_magic_safe_t magic;
    size_t i;

    for (i = 0; i < 1000; i++) {
        sprintf(magic, "%04zu", i);
        TEST_ASSERT_TRUE(pino_handler_register(magic, &g_ph_handler_spl1_obj));
    }
}

void test_register_identity(void)
{
    handler_entry_t *entry2, *entry3;
    pino_t *pino2, *pino3;
    uint8_t data[TEST_DATA_SIZE];

    generate_random_data(data, TEST_DATA_SIZE);

    TEST_ASSERT_TRUE(pino_handler_register("spl2", &g_ph_handler_spl1_obj));
    TEST_ASSERT_TRUE(pino_handler_register("spl3", &g_ph_handler_spl1_obj));

    entry2 = pino_handler_find_entry("spl2");
    entry3 = pino_handler_find_entry("spl3");

    TEST_ASSERT_NOT_NULL(entry2);
    TEST_ASSERT_NOT_NULL(entry3);
    TEST_ASSERT_NOT_EQUAL(entry2, entry3);

    pino2 = pino_pack("spl2", data, sizeof(data));
    pino3 = pino_pack("spl3", data, sizeof(data));

    TEST_ASSERT_NOT_NULL(pino2);
    TEST_ASSERT_NOT_NULL(pino3);
    TEST_ASSERT_EQUAL_PTR(entry2, pino2->entry);
    TEST_ASSERT_EQUAL_PTR(entry3, pino3->entry);
    TEST_ASSERT_EQUAL_size_t(1, entry2->refcount);
    TEST_ASSERT_EQUAL_size_t(1, entry3->refcount);

    pino_destroy(pino2);
    TEST_ASSERT_EQUAL_size_t(0, entry2->refcount);
    TEST_ASSERT_EQUAL_size_t(1, entry3->refcount);

    pino_destroy(pino3);
    TEST_ASSERT_EQUAL_size_t(0, entry3->refcount);

    TEST_ASSERT_TRUE(pino_handler_unregister("spl2"));
    TEST_ASSERT_TRUE(pino_handler_unregister("spl3"));
}

void test_pack(void)
{
    pino_t *pino;
    uint8_t *data, *unpacked_data;

    data = (uint8_t *)malloc(TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(data);

    generate_random_data(data, TEST_DATA_SIZE);

    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    TEST_ASSERT_EQUAL_size_t(TEST_DATA_SIZE, pino_unpack_size(pino));

    unpacked_data = (uint8_t *)malloc(pino_unpack_size(pino));
    TEST_ASSERT_NOT_NULL(unpacked_data);

    TEST_ASSERT_TRUE(pino_unpack(pino, unpacked_data));
    TEST_ASSERT_EQUAL_MEMORY(data, unpacked_data, TEST_DATA_SIZE);

    pino_destroy(pino);
    free(data);
    free(unpacked_data);
}

void test_pack_fail(void)
{
    pino_t *pino;
    uint8_t *data;

    data = (uint8_t *)malloc(TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(data);

    generate_random_data(data, TEST_DATA_SIZE);

    pino = pino_pack("spl2", data, TEST_DATA_SIZE);
    TEST_ASSERT_NULL(pino);

    pino = pino_pack("spl3", data, TEST_DATA_SIZE);
    TEST_ASSERT_NULL(pino);

    pino = pino_pack("spl4", data, TEST_DATA_SIZE);
    TEST_ASSERT_NULL(pino);

    free(data);
}

void test_pack_glowing(void)
{
    pino_t **pinos;
    uint8_t *data;
    size_t i;

    data = (uint8_t *)malloc(TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(data);

    generate_random_data(data, TEST_DATA_SIZE);

    pinos = (pino_t **)malloc(sizeof(pino_t *) * 1000);
    TEST_ASSERT_NOT_NULL(pinos);

    for (i = 0; i < 1000; i++) {
        pinos[i] = pino_pack("spl1", data, TEST_DATA_SIZE);
        TEST_ASSERT_NOT_NULL(pinos[i]);
    }

    for (i = 0; i < 1000; i++) {
        pino_destroy(pinos[i]);
    }

    free(data);
    free(pinos);
}

void test_pino_serialize(void)
{
    pino_t *pino, *unserialized_pino;
    uint8_t *data, *serialized_data, *unserialized_data;
    size_t serialize_size, unpack_size;

    data = (uint8_t *)malloc(TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(data);

    generate_random_data(data, TEST_DATA_SIZE);

    pino = pino_pack("spl1", data, TEST_DATA_SIZE);
    TEST_ASSERT_NOT_NULL(pino);

    serialize_size = pino_serialize_size(pino);
    TEST_ASSERT_GREATER_THAN_size_t(TEST_DATA_SIZE, serialize_size);

    serialized_data = (uint8_t *)malloc(serialize_size);
    TEST_ASSERT_NOT_NULL(serialized_data);

    TEST_ASSERT_TRUE(pino_serialize(pino, serialized_data));

    unserialized_pino = pino_unserialize(serialized_data, serialize_size);
    TEST_ASSERT_NOT_NULL(unserialized_pino);

    unpack_size = pino_unpack_size(unserialized_pino);
    TEST_ASSERT_EQUAL_size_t(TEST_DATA_SIZE, unpack_size);

    unserialized_data = (uint8_t *)malloc(unpack_size);
    TEST_ASSERT_NOT_NULL(unserialized_data);

    TEST_ASSERT_TRUE(pino_unpack(unserialized_pino, unserialized_data));
    TEST_ASSERT_EQUAL_MEMORY(data, unserialized_data, TEST_DATA_SIZE);

    pino_destroy(pino);
    pino_destroy(unserialized_pino);
    free(data);
    free(serialized_data);
    free(unserialized_data);
}

void test_typed_array_serialization(void)
{
    pino_t *pino, *restored;
    uint32_t source[2] = {0x11223344U, 0x55667788U};
    uint32_t unpacked[2] = {0, 0};
    uint8_t serialized[4 + sizeof(pino_static_fields_size_t) + sizeof(uint32_t) + sizeof(source)];
    size_t offset;

    TEST_ASSERT_TRUE(PH_REG(u32a));

    pino = pino_pack("u32a", source, sizeof(source));
    TEST_ASSERT_NOT_NULL(pino);
    TEST_ASSERT_EQUAL_size_t(sizeof(serialized), pino_serialize_size(pino));
    TEST_ASSERT_TRUE(pino_serialize(pino, serialized));

    offset = sizeof(pino_magic_t) + sizeof(pino_static_fields_size_t) + sizeof(uint32_t);
    TEST_ASSERT_EQUAL_HEX8(0x44, serialized[offset + 0]);
    TEST_ASSERT_EQUAL_HEX8(0x33, serialized[offset + 1]);
    TEST_ASSERT_EQUAL_HEX8(0x22, serialized[offset + 2]);
    TEST_ASSERT_EQUAL_HEX8(0x11, serialized[offset + 3]);
    TEST_ASSERT_EQUAL_HEX8(0x88, serialized[offset + 4]);
    TEST_ASSERT_EQUAL_HEX8(0x77, serialized[offset + 5]);
    TEST_ASSERT_EQUAL_HEX8(0x66, serialized[offset + 6]);
    TEST_ASSERT_EQUAL_HEX8(0x55, serialized[offset + 7]);

    restored = pino_unserialize(serialized, sizeof(serialized));
    TEST_ASSERT_NOT_NULL(restored);
    TEST_ASSERT_EQUAL_size_t(sizeof(unpacked), pino_unpack_size(restored));
    TEST_ASSERT_TRUE(pino_unpack(restored, unpacked));
    TEST_ASSERT_EQUAL_UINT32(source[0], unpacked[0]);
    TEST_ASSERT_EQUAL_UINT32(source[1], unpacked[1]);

    pino_destroy(restored);
    pino_destroy(pino);

    TEST_ASSERT_TRUE(PH_UNREG(u32a));
}

void test_version_id(void)
{
    TEST_ASSERT_EQUAL_UINT32(PINO_VERSION_ID, pino_version_id());
}

void test_buildtime(void)
{
    TEST_ASSERT_GREATER_THAN(0, pino_buildtime());
}

void test_file(void)
{
    pino_t *pino, *pino_unserialized;
    uint8_t *data, *serialized_data, *unpacked_data, *unserialized_unpacked_data;
    size_t size, serialized_size, unpacked_size, unserialized_unpacked_size;

    size = TEST_DATA_SIZE;
    data = (uint8_t *)malloc(size);
    TEST_ASSERT_NOT_NULL(data);
    generate_fixed_data(data, size);

    pino = pino_pack("spl1", data, size);
    TEST_ASSERT_NOT_NULL(pino);
    set_u32(pino, 123456789);
    TEST_ASSERT_EQUAL_UINT32(123456789, get_u32(pino));

    serialized_size = pino_serialize_size(pino);
    serialized_data = (uint8_t *)malloc(serialized_size);
    TEST_ASSERT_NOT_NULL(serialized_data);
    TEST_ASSERT_TRUE(pino_serialize(pino, serialized_data));

    unpacked_size = pino_unpack_size(pino);
    unpacked_data = (uint8_t *)malloc(unpacked_size);
    TEST_ASSERT_NOT_NULL(unpacked_data);
    TEST_ASSERT_TRUE(pino_unpack(pino, unpacked_data));
    TEST_ASSERT_EQUAL_MEMORY(data, unpacked_data, size);

    pino_unserialized = pino_unserialize(serialized_data, serialized_size);
    TEST_ASSERT_NOT_NULL(pino_unserialized);
    TEST_ASSERT_EQUAL_UINT32(123456789, get_u32(pino_unserialized));

    unserialized_unpacked_size = pino_unpack_size(pino_unserialized);
    TEST_ASSERT_GREATER_THAN_size_t(0, unserialized_unpacked_size);
    unserialized_unpacked_data = (uint8_t *)malloc(unserialized_unpacked_size);
    TEST_ASSERT_NOT_NULL(unserialized_unpacked_data);

    TEST_ASSERT_TRUE(pino_unpack(pino_unserialized, unserialized_unpacked_data));

    TEST_ASSERT_EQUAL_MEMORY(data, unserialized_unpacked_data, size);
    TEST_ASSERT_TRUE(save_file(g_asset_path, serialized_data, serialized_size));

    pino_destroy(pino);
    pino_destroy(pino_unserialized);
    free(unserialized_unpacked_data);
    free(serialized_data);
    free(unpacked_data);
    free(data);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_register);
    RUN_TEST(test_register_fail);
    RUN_TEST(test_register_unregistered);
    RUN_TEST(test_register_glowing);
    RUN_TEST(test_register_identity);
    RUN_TEST(test_pack);
    RUN_TEST(test_pack_fail);
    RUN_TEST(test_pack_glowing);
    RUN_TEST(test_pino_serialize);
    RUN_TEST(test_typed_array_serialization);

    RUN_TEST(test_version_id);
    RUN_TEST(test_buildtime);

    RUN_TEST(test_file);

    return UNITY_END();
}
