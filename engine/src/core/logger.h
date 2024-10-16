#pragma once

/**
 * @file
 * @brief Logger implementation
 *
 * Almost all forms of text i/o should take place through one of these loggers.
 * There are some helper macros defined to log with filenames and lines for debugging.
 * There's also a global logger instance used by these macros and should be preferred
 * except for exceptional and documented cases.
 */

#include <defines.h>
#include <stdlib.h>

#define LOG_MAX_LENGTH ((u64)30000)

enum log_level {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_FATAL = 4,
};

struct logger {
    enum log_level level;
};

LAPI void logger_message(
    struct logger *logger, 
    enum log_level level, 
    const char *fmt, 
    ...
) LHINT_FORMAT(3, 4);

#define LDEBUG(fmt, ...) \
    logger_message(&GLOBAL_LOGGER, LOG_LEVEL_DEBUG, "[" LFILE_NAME ":%d] Debug: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LINFO(fmt, ...) \
    logger_message(&GLOBAL_LOGGER, LOG_LEVEL_INFO, "[" LFILE_NAME ":%d] Info: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LWARN(fmt, ...) \
    logger_message(&GLOBAL_LOGGER, LOG_LEVEL_WARNING, "[" LFILE_NAME ":%d] Warning: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LERROR(fmt, ...) \
    logger_message(&GLOBAL_LOGGER, LOG_LEVEL_ERROR, "[" LFILE_NAME ":%d] Error: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LFATAL(fmt, ...) \
    do { \
        logger_message(&GLOBAL_LOGGER, LOG_LEVEL_FATAL, "[" LFILE_NAME ":%d] Fatal: " fmt "\n", __LINE__, ##__VA_ARGS__); \
        abort(); \
    } while(0)

extern struct logger GLOBAL_LOGGER;
