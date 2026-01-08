/*
 * libpino - portable.c
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <pino/portable.h>

#if defined(_MSC_VER)
#include <stdlib.h>
#endif

extern uint16_t pino_bswap16(uint16_t x) {
  return ((x & 0xff) << 8) | ((x & 0xff00) >> 8);
}

extern uint32_t pino_bswap32(uint32_t x) {
#if defined(_MSC_VER)
  return _byteswap_ulong(x);
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap32(x);
#else
  return ((x & 0xff) << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) |
         ((x & 0xff000000) >> 24);
#endif
}

extern uint64_t pino_bswap64(uint64_t x) {
#if defined(_MSC_VER)
  return _byteswap_uint64(x);
#elif defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap64(x);
#else
  return ((x & 0xff) << 56) | ((x & 0xff00) << 40) | ((x & 0xff0000) << 24) |
         ((x & 0xff000000) << 8) | ((x & 0xff00000000ULL) >> 8) |
         ((x & 0xff0000000000ULL) >> 24) | ((x & 0xff000000000000ULL) >> 40) |
         ((x & 0xff00000000000000ULL) >> 56);
#endif
}
