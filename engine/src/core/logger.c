#include "logger.h"

#include <platform/platform.h>
#include <stdarg.h>
#include <stdio.h>

struct logger GLOBAL_LOGGER;

LAPI void logger_message(struct logger *logger, enum log_level level, const char *fmt, ...)
{
    // TODO - platform specific print for colors
    
    if (level < logger->level) {
        return;
    }
    
    enum terminal_color color;
    switch (level) {
    case LOG_LEVEL_DEBUG:
        color = TERMINAL_COLOR_GRAY;
        break;
    case LOG_LEVEL_INFO:
        color = TERMINAL_COLOR_GREEN;
        break;
    case LOG_LEVEL_WARNING:
        color = TERMINAL_COLOR_YELLOW;
        break;
    case LOG_LEVEL_ERROR:
        color = TERMINAL_COLOR_RED;
        break;
    case LOG_LEVEL_FATAL:
        color = TERMINAL_COLOR_PURPLE;
        break;
    }

    char buffer[32000]; // please dont use all of this
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    platform_print_color(stdout, buffer, color);
}
