#pragma once

#include <defines.h>
#include <stdlib.h>

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

LAPI void log_message(struct logger *logger, enum log_level level, const char *fmt, ...);

#define LDEBUG(fmt, ...) \
    log_message(&GLOBAL_LOGGER, LOG_LEVEL_DEBUG, "["__FILE__":%d] Debug: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LINFO(fmt, ...) \
    log_message(&GLOBAL_LOGGER, LOG_LEVEL_INFO, "["__FILE__":%d] Info: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LWARN(fmt, ...) \
    log_message(&GLOBAL_LOGGER, LOG_LEVEL_WARNING, "["__FILE__":%d] Warning: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LERROR(fmt, ...) \
    log_message(&GLOBAL_LOGGER, LOG_LEVEL_ERROR, "["__FILE__":%d] Error: " fmt "\n", __LINE__, ##__VA_ARGS__)

#define LFATAL(fmt, ...) \
    do { \
        log_message(&GLOBAL_LOGGER, LOG_LEVEL_FATAL, "["__FILE__":%d] Fatal: " fmt "\n", __LINE__, ##__VA_ARGS__); \
        abort(); \
    } while(0)

extern struct logger GLOBAL_LOGGER;
