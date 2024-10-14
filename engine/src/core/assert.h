#pragma once

#include "logger.h"

// TODO: Windows version
#include <signal.h>
#define debug_break() raise(SIGTRAP)

#define LASSERT(expr)                                                   \
    do {                                                                \
        if (!(expr)) {                                                  \
            LERROR("Assertion Failure: " #expr);                        \
            debug_break();                                              \
        }                                                               \
    } while(0)

#define LASSERT_MSG(expr, msg)                                          \
    do {                                                                \
        if (!(expr)) {                                                  \
            LERROR("Assertion Failure: " #expr ", message " msg);       \
            debug_break();                                              \
        }                                                               \
    } while(0)
