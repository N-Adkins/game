#pragma once

/**
 * @file
 * @brief Assertions
 *
 * If the expression in an assertion returns false, it will print the expression through
 * the logging system and attempt to cause a debug breakpoint, otherwise it will abort.
 */

#include <core/logger.h>
#include <platform/platform.h>
#include <defines.h>

// clang-format off

#ifdef LENGINE_DEBUG

#define LASSERT(expr)                                                           \
	do {                                                                        \
		if (!(expr)) {                                                          \
			LERROR("Assertion Failure: " #expr);                                \
			platform_debug_break();                                             \
		}                                                                       \
	} while (0)

#define LASSERT_MSG(expr, msg)                                                  \
	do {                                                                        \
		if (!(expr)) {                                                          \
			LERROR("Assertion Failure: " #expr ", message " msg);               \
			platform_debug_break();                                             \
		}                                                                       \
	} while (0)

#else 
#define LASSERT(expr) ((void)0)
#define LASSERT_MSG(expr, msg) ((void)0)
#endif

/**
 * @brief Errors at compile time if expr is false at compile time
 *
 * This uses __COUNTER__ to attempt to make every static_assertion unique.
 * We could use LUNIQUE_NAME(0) but it's better to have the name have 
 * "static_assertion_failed" in it for clarity on error.
 *
 * Avoid using this in headers, if it fails then it will spit out an error
 * for every translation that includes the header.
 */
#define LSTATIC_ASSERT(expr, msg)                                       \
	typedef char LCONCAT(static_assertion_failed_,                      \
			     __COUNTER__)[(expr) ? 1 : -1]

// clang-format on
