#pragma once

#include <platform/platform.h>
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
