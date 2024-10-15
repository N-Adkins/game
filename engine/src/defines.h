#pragma once

/**
 * @file
 * @brief Defines
 *
 * Defines some very common macros and types to be used. These include fixed-width
 * integer type definitions as well as symbol exporting.
 */

#include <stdint.h>

// TODO Add windows definition
#define LAPI

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef float       f32;
typedef double      f64;
typedef u8          b8;

#define true    ((b8)1)
#define false   ((b8)0)

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
