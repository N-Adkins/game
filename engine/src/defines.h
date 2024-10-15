#pragma once

/**
 * @file
 * @brief Defines
 *
 * Defines some very common macros and types to be used. These include fixed-width
 * integer type definitions as well as symbol exporting.
 */

#include <stdint.h>

// TODO: other platforms lol
#define LPLATFORM_LINUX

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

/**
 * @brief Tells the compiler a function is using printf for warnings
 *
 * This uses compiler-specific extensions to tell some compilers that a function is
 * passing the arguments to a formatting function so that warnings such as -Wformat
 * properly warn on them.
 *
 * Ex. 
 * void foo_printf(int bar, const char *fmt, ...) FORMAT_HINT(2, 3)
 *
 * @param fmt_index Arg number that is the format string
 * @param arg_index Arg number that is variable argument indicator
 */
#ifdef LPLATFORM_LINUX
#define LHINT_FORMAT(fmt_index, arg_index) \
    __attribute__((format(printf, fmt_index, arg_index)));
#else
#define LHINT_FORMAT(fmt_index, arg_index)
#endif

/**
 * @brief Following function will always be inlined
 *
 * This really should not be used unless there is a measurable reason to, you aren't 
 * smarter than the compiler. An example of a place that it could reasonably
 * be used is something like a SIMD wrapper where call overhead would completely
 * invalidate the benefits of using SIMD.
 */
#ifdef LPLATFORM_LINUX
#define LHINT_INLINE \
    __attribute__((always_inline))
#else
#define LHINT_INLINE
#endif
