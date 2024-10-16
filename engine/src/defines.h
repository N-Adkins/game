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

typedef uint8_t                     u8;
typedef uint16_t                    u16;
typedef uint32_t                    u32;
typedef unsigned long long int      u64;
typedef int8_t                      i8;
typedef int16_t                     i16;
typedef int32_t                     i32;
typedef signed long long int        i64;
typedef float                       f32;
typedef double                      f64;
typedef _Bool                       b8;

#define true    ((b8)1)
#define false   ((b8)0)

/**
 * @brief Returns the number of elements in a statically-sized array.
 */
#define LARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/**
 * @brief Returns the number of elements in a staticially-sized string, such
 * as an array.
 */
#define LSTRING_LENGTH(x) (LARRAY_LENGTH((x)-1)

/**
 * @brief Commonly used token concat macro, used for making other macros.
 */
#define LCONCAT(x, y) LCONCAT_HELPER(x, y)
#define LCONCAT_HELPER(x, y) x##y

/**
 * @brief Commonly used token stringify macro, turns a token sequence 
 * into a string litera.
 */
#define LSTRINGIFY(x) LSTRINGIFY_HELPER(x)
#define LSTRINGIFY_HELPER(x) #x

#if defined(__clang__)
#define LCOMPILER_CLANG
#define LCOMPILER "Clang"
#elif defined(__GNUC__) || defined(__GNUG__)
#define LCOMPILER_GCC
#define LCOMPILER "GCC"
#elif defined(_MSC_VER)
#define LCOMPILER_MSVC
#define LCOMPILER "MSVC"
#else
#error Unsupported compiler, expecting GCC, Clang, or MSVC
#endif

// This just exists since lots of extensions work on both, saves some typing
#if defined(LCOMPILER_CLANG) || defined(LCOMPILER_GCC)
#define LCOMPILER_CLANG_OR_GCC
#endif

#if defined(__linux__)
#define LPLATFORM_LINUX
#define LPLATFORM "Linux"
#elif defined(_WIN32)
#define LPLATFORM_WINDOWS
#define LPLATFORM "Windows"
#elif defined (__APPLE__)
/#define LPLATFORM_MACOS
#define LPLATFORM "MacOS"
#else
#error Unsupported platform
#endif

/**
 * @brief Symbol exporting macro
 *
 * This should be used on any function that should be available in the
 * application executable. Note that forgetting this won't do anything
 * on any platform except Windows, which is the only one where it is
 * required. This relies on a macro passed in by the build system
 * specifiying whether it is an engine build or not.
 */
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
 * @brief Generates a compiler warning with the passed message.
 */
#ifdef LCOMPILER_CLANG_OR_GCC
#define LCOMPILE_WARN(msg) \
    _Pragma(LSTRINGIFY(message(msg)))
#else // MSVC
#define LCOMPILE_WARN(msg) \
    __pragma(message(msg))
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
#else // MSVC
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
#else // MSVC
#define LHINT_INLINE \
    __forceinline
#endif

/**
 * @brief Returns the current function name as a string literal
 */
#ifdef LCOMPILER_CLANG_OR_GCC
#define LFUNCTION \
    __func__
#else // MSVC
#define LFUNCTION \
    __FUNCTION__
#endif

/**
 * @brief Returns the type of the passed expression as a token
 */
#ifdef LCOMPILER_CLANG_OR_GCC
#define LTYPEOF(expr) \
    typeof(expr)
#else // MSVC
#define LTYPEOF(expr) \
    __typeof__(expr)
#endif

/**
 * @brief Generates a unique identifier for each passed number in a macro context
 *
 * This should be used sparingly, it's for more complicated macros where you need
 * scoped variables. It relies on __LINE__. Calling this with the same passed number
 * within the same macro body will return the same result so it can be reused. This will
 * not have consistent results if you are doing some weird stuff like nested expression
 * statements as the line would be the same, this is for function-like macros that use 
 * do {} while(0) and the like.
 */
#define LUNIQUE_ID(num) \
    LCONCAT(_unique_id_, LCONCAT(__LINE__, LCONCAT(_ver_, num)))
