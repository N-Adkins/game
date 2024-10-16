#pragma once

/**
 * @file
 * @brief Assertions
 *
 * If the expression in an assertion returns false, it will print the expression through
 * the logging system and attempt to cause a debug breakpoint, otherwise it will abort.
 */

#include <platform/platform.h>
#include <defines.h>
#include "logger.h"

#define LASSERT(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            LERROR("Assertion Failure: " #expr);                        \
            platform_debug_break();                                     \
        }                                                               \
    } while(0)

#define LASSERT_MSG(expr, msg)                                          \
    do {                                                                \
        if (!(expr)) {                                                  \
            LERROR("Assertion Failure: " #expr ", message " msg);       \
            platform_debug_break();                                     \
        }                                                               \
    } while(0)

#define LSTATIC_ASSERT(expr, msg) \
    typedef char LUNIQUE_ID(0)[(expr)?1:-1]
