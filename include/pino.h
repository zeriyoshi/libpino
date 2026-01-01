/*
 * libpino header - pino.h
 * 
 */

#ifndef PINO_H
#define PINO_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t pino_buildtime_t;

typedef struct _pino_handler_t pino_handler_t;

typedef char pino_magic_t[4];
typedef char pino_magic_safe_t[sizeof(pino_magic_t) + 1];  /* + '\0' */

typedef uint64_t pino_static_fields_size_t;

typedef struct {
    pino_magic_safe_t magic;
    pino_static_fields_size_t static_fields_size;
    pino_handler_t *handler;
    void *static_fields;
    void *this;
} pino_t;

bool pino_init(void);
void pino_free(void);

size_t pino_serialize_size(const pino_t *pino);
bool pino_serialize(const pino_t *pino, void *dest);
pino_t *pino_unserialize(const void *src, size_t size);
pino_t *pino_pack(pino_magic_safe_t magic, const void *src, size_t size);
size_t pino_unpack_size(const pino_t *pino);
bool pino_unpack(const pino_t *pino, void *dest);
void pino_destroy(pino_t *pino);

uint32_t pino_version_id(void);
pino_buildtime_t pino_buildtime(void);

#ifdef __cplusplus
}
#endif

#endif  /* PINO_H */
