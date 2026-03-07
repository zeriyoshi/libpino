/*
 * libpino - simd.h
 *
 * This file is part of libpino.
 *
 * Author: Go Kudo <zeriyoshi@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef PINO_INTERNAL_SIMD_H
#define PINO_INTERNAL_SIMD_H

#include <stddef.h>
#include <stdint.h>

#if defined(PINO_SIMD_AVX2)
#include <immintrin.h>
#elif defined(PINO_SIMD_NEON)
#include <arm_neon.h>
#elif defined(PINO_SIMD_WASM)
#include <wasm_simd128.h>
#endif

#endif /* PINO_INTERNAL_SIMD_H */
