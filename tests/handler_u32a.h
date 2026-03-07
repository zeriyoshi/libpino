/*
 * libpino - handler_u32a.h
 *
 * This file is part of libpino.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PINO_TESTS_HANDLER_U32A_H
#define PINO_TESTS_HANDLER_U32A_H

#include <stddef.h>
#include <stdint.h>

#include <pino.h>
#include <pino/handler.h>

PH_BEGIN(u32a);

PH_DEF_STATIC_FIELDS_STRUCT(u32a)
{
    uint32_t count;
}
PH_DEF_STATIC_FIELDS_STRUCT_END;

PH_DEF_STRUCT(u32a)
{
    uint32_t words[2];
}
PH_DEF_STRUCT_END;

PH_DEFUN_SERIALIZE_SIZE(u32a)
{
    return sizeof(PH_THIS(u32a)->words);
}

PH_DEFUN_SERIALIZE(u32a)
{
    PH_SERIALIZE_DATA(u32a, words, sizeof(PH_THIS(u32a)->words));
    return true;
}

PH_DEFUN_UNSERIALIZE(u32a)
{
    PH_UNSERIALIZE_DATA(u32a, words, sizeof(PH_THIS(u32a)->words));
    return true;
}

PH_DEFUN_PACK(u32a)
{
    uint32_t count = 2;

    if (PH_ARG_SIZE != sizeof(PH_THIS(u32a)->words)) {
        return false;
    }

    PH_THIS_STATIC_SET(u32a, count, &count);
    PH_PACK_DATA(u32a, words, sizeof(PH_THIS(u32a)->words));
    return true;
}

PH_DEFUN_UNPACK_SIZE(u32a)
{
    return sizeof(PH_THIS(u32a)->words);
}

PH_DEFUN_UNPACK(u32a)
{
    PH_UNPACK_DATA(u32a, words, sizeof(PH_THIS(u32a)->words));
    return true;
}

PH_DEFUN_CREATE(u32a)
{
    uint32_t count = 2;

    PH_CREATE_THIS(u32a);

    if (PH_ARG_SIZE != sizeof(PH_THIS(u32a)->words)) {
        PH_DESTROY_THIS(u32a);
        return NULL;
    }

    PH_THIS_STATIC_SET(u32a, count, &count);

    return PH_THIS(u32a);
}

PH_DEFUN_DESTROY(u32a)
{
    PH_DESTROY_THIS(u32a);
}

PH_END(u32a);

#endif
