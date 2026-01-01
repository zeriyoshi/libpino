/*
 * libpino header - pino/handler.h
 * 
 */

#ifndef PINO_HANDLER_H
#define PINO_HANDLER_H

#include <string.h>

#include <pino.h>
#include <pino/endianness.h>

#ifdef __cplusplus
extern "C" {
#endif

bool pino_handler_register(pino_magic_safe_t magic, pino_handler_t *handler);
bool pino_handler_unregister(pino_magic_safe_t magic);

void *pino_memory_manager_malloc(void *entry, size_t size);
void *pino_memory_manager_calloc(void *entry, size_t count, size_t size);
void pino_memory_manager_free(void *entry, void *ptr);

#define PH_NAME_HANDLER(name)                           g_ph_handler_##name##_obj
#define PH_NAME_REG(name)                               _ph_handler_##name##_register
#define PH_NAME_UNREG(name)                             _ph_handler_##name##_unregister
#define PH_NAME_STRUCT(name)                            _ph_handler_##name##_struct
#define PH_NAME_STATIC_FIELDS_STRUCT(name)              _ph_handler_##name##_static_fields_struct
#define PH_NAME_FUNC_SERIALIZE_SIZE(name)               _ph_handler_##name##_serialize_size
#define PH_NAME_FUNC_SERIALIZE(name)                    _ph_handler_##name##_serialize
#define PH_NAME_FUNC_UNSERIALIZE(name)                  _ph_handler_##name##_unserialize
#define PH_NAME_FUNC_PACK(name)                         _ph_handler_##name##_pack
#define PH_NAME_FUNC_UNPACK_SIZE(name)                  _ph_handler_##name##_unpack_size
#define PH_NAME_FUNC_UNPACK(name)                       _ph_handler_##name##_unpack
#define PH_NAME_FUNC_CREATE(name)                       _ph_handler_##name##_create
#define PH_NAME_FUNC_DESTROY(name)                      _ph_handler_##name##_destroy

#define PH_ARG_THIS                                     __this
#define PH_ARG_DATA                                     __data
#define PH_ARG_SIZE                                     __size
#define PH_ARG_SRC                                      __src
#define PH_ARG_SRC_SIZE                                 __src_size
#define PH_ARG_DST                                      __dest
#define PH_ARG_STATIC_FIELDS                            __static_fields

#define PH_SIGNATURE_SERIALIZE_SIZE                     (const void *PH_ARG_THIS, const void *PH_ARG_STATIC_FIELDS)
#define PH_SIGNATURE_SERIALIZE                          (const void *PH_ARG_THIS, const void *PH_ARG_STATIC_FIELDS, void *PH_ARG_DST)
#define PH_SIGNATURE_UNSERIALIZE                        (void *PH_ARG_THIS, void *PH_ARG_STATIC_FIELDS, const void *PH_ARG_SRC, size_t PH_ARG_SRC_SIZE)
#define PH_SIGNATURE_PACK                               (void *PH_ARG_THIS, void *PH_ARG_STATIC_FIELDS, const void *PH_ARG_SRC, size_t PH_ARG_SIZE)
#define PH_SIGNATURE_UNPACK_SIZE                        (const void *PH_ARG_THIS, const void *PH_ARG_STATIC_FIELDS)
#define PH_SIGNATURE_UNPACK                             (const void *PH_ARG_THIS, const void *PH_ARG_STATIC_FIELDS, void *PH_ARG_DST)
#define PH_SIGNATURE_CREATE                             (size_t PH_ARG_SIZE, void *PH_ARG_STATIC_FIELDS)
#define PH_SIGNATURE_DESTROY                            (void *PH_ARG_THIS, void *PH_ARG_STATIC_FIELDS)

#if defined(_MSC_VER)
# define PH_DEF_STRUCT(name)                            __pragma(pack(push, 1)) struct PH_NAME_STRUCT(name)
# define PH_DEF_STRUCT_END                              ; __pragma(pack(pop))
# define PH_DEF_STATIC_FIELDS_STRUCT(name)              __pragma(pack(push, 1)) struct PH_NAME_STATIC_FIELDS_STRUCT(name)
# define PH_DEF_STATIC_FIELDS_STRUCT_END                ; __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
# define PH_DEF_STRUCT(name)                            struct __attribute__((packed)) PH_NAME_STRUCT(name)
# define PH_DEF_STRUCT_END                              ;
# define PH_DEF_STATIC_FIELDS_STRUCT(name)              struct __attribute__((packed)) PH_NAME_STATIC_FIELDS_STRUCT(name)
# define PH_DEF_STATIC_FIELDS_STRUCT_END                ;
#else
# define PH_DEF_STRUCT(name)                            struct PH_NAME_STRUCT(name)
# define PH_DEF_STRUCT_END                              ;
# define PH_DEF_STATIC_FIELDS_STRUCT(name)              struct PH_NAME_STATIC_FIELDS_STRUCT(name)
# define PH_DEF_STATIC_FIELDS_STRUCT_END                ;
# warning "Unknown compiler, struct packing may not work correctly"
#endif

#define PH_DEFUN_SERIALIZE_SIZE(name)                   static size_t PH_NAME_FUNC_SERIALIZE_SIZE(name)PH_SIGNATURE_SERIALIZE_SIZE
#define PH_DEFUN_SERIALIZE(name)                        static bool PH_NAME_FUNC_SERIALIZE(name)PH_SIGNATURE_SERIALIZE
#define PH_DEFUN_UNSERIALIZE(name)                      static bool PH_NAME_FUNC_UNSERIALIZE(name)PH_SIGNATURE_UNSERIALIZE
#define PH_DEFUN_PACK(name)                             static bool PH_NAME_FUNC_PACK(name)PH_SIGNATURE_PACK
#define PH_DEFUN_UNPACK_SIZE(name)                      static size_t PH_NAME_FUNC_UNPACK_SIZE(name)PH_SIGNATURE_UNPACK_SIZE
#define PH_DEFUN_UNPACK(name)                           static bool PH_NAME_FUNC_UNPACK(name)PH_SIGNATURE_UNPACK
#define PH_DEFUN_CREATE(name)                           static void *PH_NAME_FUNC_CREATE(name)PH_SIGNATURE_CREATE
#define PH_DEFUN_DESTROY(name)                          static void PH_NAME_FUNC_DESTROY(name)PH_SIGNATURE_DESTROY

#define PH_THIS_P(name, ptr)                            ((struct PH_NAME_STRUCT(name) *)ptr)
#define PH_THIS_STATIC_P(name, ptr)                     ((struct PH_NAME_STATIC_FIELDS_STRUCT(name) *)ptr)
#define PH_THIS_STATIC_GET_P(name, ptr, param, dest)    do { \
    PH_MEMCPY_L2N(dest, &PH_THIS_STATIC_P(name, ptr)->param, sizeof(PH_THIS_STATIC_P(name, ptr)->param)); \
} while (0)
#define PH_THIS_STATIC_SET_P(name, ptr, param, src)     do { \
    PH_MEMCPY_N2L(&PH_THIS_STATIC_P(name, ptr)->param, src, sizeof(PH_THIS_STATIC_P(name, ptr)->param)); \
} while (0)
#define PH_THIS(name)                                   (PH_THIS_P(name, PH_ARG_THIS))
#define PH_THIS_STATIC(name)                            (PH_THIS_STATIC_P(name, PH_ARG_STATIC_FIELDS))
#define PH_THIS_STATIC_GET(name, param, dest)           PH_THIS_STATIC_GET_P(name, PH_ARG_STATIC_FIELDS, param, dest)
#define PH_THIS_STATIC_SET(name, param, src)            PH_THIS_STATIC_SET_P(name, PH_ARG_STATIC_FIELDS, param, src)
#define PH_PINO_P(name, pino)                           (PH_THIS_P(name, ((pino_t *)pino)->this))
#define PH_PINO_STATIC_P(name, pino)                    ((struct PH_NAME_STATIC_FIELDS_STRUCT(name) *)pino->static_fields)
#define PH_PINO_STATIC_GET_P(name, pino, param, dest)   PH_THIS_STATIC_GET_P(name, PH_PINO_STATIC_P(name, pino), param, dest)
#define PH_PINO_STATIC_SET_P(name, pino, param, src)    PH_THIS_STATIC_SET_P(name, PH_PINO_STATIC_P(name, pino), param, src)

#define PH_SIZE(name)                                   (sizeof(struct PH_NAME_STRUCT(name)))
#define PH_SIZE_STATIC(name)                            (sizeof(struct PH_NAME_STATIC_FIELDS_STRUCT(name)))

#define PH_MALLOC(name, size)                           pino_memory_manager_malloc(PH_NAME_HANDLER(name).entry, size)
#define PH_CALLOC(name, count, size)                    pino_memory_manager_calloc(PH_NAME_HANDLER(name).entry, count, size)
#define PH_FREE(name, ptr)                              pino_memory_manager_free(PH_NAME_HANDLER(name).entry, ptr)

#define PH_MEMCPY(dst, src, size)                       memcpy(dst, src, size)
#define PH_MEMCPY_N2L(dst, src, size)                   pino_endianness_memcpy_native2le(dst, src, size)
#define PH_MEMCPY_N2B(dst, src, size)                   pino_endianness_memcpy_native2be(dst, src, size)
#define PH_MEMCPY_L2N(dst, src, size)                   pino_endianness_memcpy_le2native(dst, src, size)
#define PH_MEMCPY_B2N(dst, src, size)                   pino_endianness_memcpy_be2native(dst, src, size)

#define PH_REG(name)                                    PH_NAME_REG(name)()
#define PH_UNREG(name)                                  PH_NAME_UNREG(name)()

#define PH_CREATE_THIS(name) \
    void *PH_ARG_THIS = PH_CALLOC(name, 1, PH_SIZE(name)); \
    if (!PH_THIS(name)) { \
        return NULL; \
    }

#define PH_DESTROY_THIS(name)                           PH_FREE(name, PH_ARG_THIS)

#define PH_SERIALIZE_DATA(name, src, size)      do { \
    PH_MEMCPY_N2L(PH_ARG_DST, PH_THIS(name)->src, size); \
} while (0)
#define PH_UNSERIALIZE_DATA(name, dest, size)   do { \
    if (size > PH_ARG_SRC_SIZE) { \
        return false; \
    } \
    PH_MEMCPY_L2N(PH_THIS(name)->dest, PH_ARG_SRC, size); \
} while (0)
#define PH_PACK_DATA(name, param, size)         do { \
    PH_MEMCPY_N2L(PH_THIS(name)->param, PH_ARG_SRC, size); \
} while (0)
#define PH_UNPACK_DATA(name, param, size)       do { \
    PH_MEMCPY_L2N(PH_ARG_DST, PH_THIS(name)->param, size); \
} while (0)

#define PH_BEGIN(name) \
    static pino_handler_t PH_NAME_HANDLER(name); \
    PH_DEFUN_SERIALIZE_SIZE(name); \
    PH_DEFUN_SERIALIZE(name); \
    PH_DEFUN_UNSERIALIZE(name); \
    PH_DEFUN_PACK(name); \
    PH_DEFUN_UNPACK_SIZE(name); \
    PH_DEFUN_UNPACK(name); \
    PH_DEFUN_CREATE(name); \
    PH_DEFUN_DESTROY(name);

#define PH_END(name) \
    static pino_handler_t PH_NAME_HANDLER(name) = { \
        .static_fields_size = PH_SIZE_STATIC(name), \
        .serialize_size = PH_NAME_FUNC_SERIALIZE_SIZE(name), \
        .serialize = PH_NAME_FUNC_SERIALIZE(name), \
        .unserialize = PH_NAME_FUNC_UNSERIALIZE(name), \
        .pack = PH_NAME_FUNC_PACK(name), \
        .unpack_size = PH_NAME_FUNC_UNPACK_SIZE(name), \
        .unpack = PH_NAME_FUNC_UNPACK(name), \
        .create = PH_NAME_FUNC_CREATE(name), \
        .destroy = PH_NAME_FUNC_DESTROY(name), \
        .entry = NULL \
    }; \
    static inline bool PH_NAME_REG(name)(void) { \
        return pino_handler_register(#name, &PH_NAME_HANDLER(name)); \
    } \
    static inline bool PH_NAME_UNREG(name)(void) { \
        return pino_handler_unregister(#name); \
    }

typedef size_t (*pino_handler_serialize_size_t)PH_SIGNATURE_SERIALIZE_SIZE;
typedef bool (*pino_handler_serialize_t)PH_SIGNATURE_SERIALIZE;
typedef bool (*pino_handler_unserialize_t)PH_SIGNATURE_UNSERIALIZE;
typedef bool (*pino_handler_pack_t)PH_SIGNATURE_PACK;
typedef size_t (*pino_handler_unpack_size_t)PH_SIGNATURE_UNPACK_SIZE;
typedef bool (*pino_handler_unpack_t)PH_SIGNATURE_UNPACK;
typedef void *(*pino_handler_create_t)PH_SIGNATURE_CREATE;
typedef void (*pino_handler_destroy_t)PH_SIGNATURE_DESTROY;

struct _pino_handler_t {
    pino_static_fields_size_t static_fields_size;
    pino_handler_serialize_size_t serialize_size;
    pino_handler_serialize_t serialize;
    pino_handler_unserialize_t unserialize;
    pino_handler_pack_t pack;
    pino_handler_unpack_size_t unpack_size;
    pino_handler_unpack_t unpack;
    pino_handler_create_t create;
    pino_handler_destroy_t destroy;
    void *entry;
};

#ifdef __cplusplus
}
#endif

#endif  /* PINO_HANDLER_H */
