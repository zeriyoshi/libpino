/*
 * libpino - portable.h
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef PINO_PORTABLE_H
#define PINO_PORTABLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t pino_bswap16(uint16_t x);
uint32_t pino_bswap32(uint32_t x);
uint64_t pino_bswap64(uint64_t x);

#ifdef __cplusplus
}
#endif

#endif /* PINO_PORTABLE_H */
