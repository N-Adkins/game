#pragma once

/**
 * @file
 * @brief Defines
 *
 * Defines some very common macros and types to be used. These include fixed-width
 * integer type definitions as well as symbol exporting. This file also handles a lot
 * of the definitions for the current platform and compiler.
 */

#include <stdint.h>

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

#if defined(__clang__)
#define LCOMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define LCOMPILER_GCC
#elif defined(_MSC_VER)
#define LCOMPILER_MSVC
#else
#error Unsupported compiler, expecting GCC, Clang, or MSVC
#endif

// This just exists since lots of extensions work on both, saves some typing
#define LCOMPILER_CLANG_OR_GCC \
    defined(LCOMPILER_CLANG) || defined(LCOMPILER_GCC)

#if defined(__linux__)
#define LPLATFORM_LINUX
#elif defined(_WIN32)
#define LPLATFORM_WINDOWS
#error Windows currently not supported
#elif defined (__APPLE__)
#define LPLATFORM_MACOS
#error MacOS currently not supported
#else
#error Unsupported platform
#endif

#ifdef LPLATFORM_WINDOWS
#ifdef LENGINE_BUILD
#define LAPI __declspec(dllexport)
#else
#define LAPI __declspec(dllimport)
#endif
#else
#define LAPI
#endif

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
#ifdef LCOMPILER_CLANG_OR_GCC
#define LHINT_FORMAT(fmt_index, arg_index) \
    __attribute__((format(printf, fmt_index, arg_index)));
#else
#warn LHINT_FORMAT not supported on this compiler
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
#ifdef LCOMPILER_CLANG_OR_GCC
#define LHINT_INLINE \
    __attribute__((always_inline))
#else
#warn LHINT_INLINE not supported on this compiler
#define LHINT_INLINE
#endif

/**
 * Returns the name of a file on supported compilers, otherwise the path to 
 * the file of unspecified length (it depends on how the build system is 
 * configured at the time).
 */
#ifdef LCOMPILER_CLANG_OR_GCC
#define LFILE_NAME \
    __FILE_NAME__
#else
#define LFILE_NAME \
    __FILE__
#endif
