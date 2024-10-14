#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

struct logger GLOBAL_LOGGER = {
    .level = LOG_LEVEL_DEBUG,
};

LAPI void log_message(struct logger *logger, enum log_level level, const char *fmt, ...)
{
    // TODO - platform specific print for colors
    
    if (level < logger->level) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
