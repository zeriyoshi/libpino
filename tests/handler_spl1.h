/*
 * libpino tests - handler_spl1.h
 * 
 */

#ifndef PINO_TESTS_HANDLER_SPL1_H
#define PINO_TESTS_HANDLER_SPL1_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <pino.h>
#include <pino/handler.h>

typedef uint32_t spl1_size_t;

PH_BEGIN(spl1);

PH_DEF_STATIC_FIELDS_STRUCT(spl1) {
    spl1_size_t size;
    uint32_t u32;
} PH_DEF_STATIC_FIELDS_STRUCT_END;

PH_DEF_STRUCT(spl1) {
    uint8_t *data;
} PH_DEF_STRUCT_END;

PH_DEFUN_SERIALIZE_SIZE(spl1) {
    spl1_size_t serialize_size;

    PH_THIS_STATIC_GET(spl1, size, &serialize_size);

    return (size_t)serialize_size;
}

PH_DEFUN_SERIALIZE(spl1) {
    spl1_size_t serialize_size;

    PH_THIS_STATIC_GET(spl1, size, &serialize_size);
    PH_SERIALIZE_DATA(spl1, data, (size_t)serialize_size);

    return true;
}

PH_DEFUN_UNSERIALIZE(spl1) {
    spl1_size_t unserialize_size;

    PH_THIS_STATIC_GET(spl1, size, &unserialize_size);
    PH_UNSERIALIZE_DATA(spl1, data, (size_t)unserialize_size);

    return PH_THIS(spl1);
}

PH_DEFUN_PACK(spl1) {
    spl1_size_t pack_size;

    PH_THIS_STATIC_GET(spl1, size, &pack_size);
    PH_PACK_DATA(spl1, data, (size_t)pack_size);

    return PH_THIS(spl1);
}

PH_DEFUN_UNPACK_SIZE(spl1) {
    spl1_size_t unpack_size;

    PH_THIS_STATIC_GET(spl1, size, &unpack_size);

    return (size_t)unpack_size;
}

PH_DEFUN_UNPACK(spl1) {
    spl1_size_t unpack_size;

    PH_THIS_STATIC_GET(spl1, size, &unpack_size);
    PH_UNPACK_DATA(spl1, data, (size_t)unpack_size);

    return true;
}

PH_DEFUN_CREATE(spl1) {
    spl1_size_t data_size = (spl1_size_t)PH_ARG_SIZE;

    PH_CREATE_THIS(spl1);

    /* for memory manager test */
    PH_MALLOC(spl1, 0);
    PH_FREE(spl1, NULL);
    PH_THIS(spl1)->data = (uint8_t *)PH_MALLOC(spl1, (size_t)data_size);
    PH_FREE(spl1, PH_THIS(spl1)->data);

    PH_THIS(spl1)->data = (uint8_t *)PH_CALLOC(spl1, 1, (size_t)data_size);
    if (!PH_THIS(spl1)->data) {
        PH_DESTROY_THIS(spl1);
        return NULL;
    }

    PH_THIS_STATIC_SET(spl1, size, &data_size);

    return PH_THIS(spl1);
}

PH_DEFUN_DESTROY(spl1) {
    PH_FREE(spl1, PH_THIS(spl1)->data);
    /* Accidentally memleak, but it's destroyed by memory manager.
    PH_FREE(spl1, PH_THIS(spl1)); 
    */
}

PH_END(spl1);

extern void set_u32(pino_t *pino, uint32_t u32val)
{
    PH_PINO_STATIC_SET_P(spl1, pino, u32, &u32val);
}

extern uint32_t get_u32(pino_t *pino)
{
    uint32_t u32val;

    PH_PINO_STATIC_GET_P(spl1, pino, u32, &u32val);

    return u32val;
}

#endif  /* PINO_TESTS_HANDLER_SPL1_H */
