#include <pch.hpp>

#include <cstring>
#include <iostream>
#include "platform.hpp"

#if defined (GAME_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#define OOF_IMPL
#include <oof.h>

#if defined (GAME_COMPILER_CLANG)
#pragma GCC diagnostic pop
#endif

namespace Engine::Log {

Logger GLOBAL_LOGGER = Logger();

std::string_view truncate_path(const std::source_location& source)
{
    const char* full = source.file_name();
    const char* src_pos = std::strstr(full, "/src/");

    if (src_pos) {
        return src_pos + 5;
    } else {
        return full;
    }
}

void Logger::log(Severity severity, const std::string& message, const std::source_location& source)
{
    std::string message_prefix = std::format("[{}:{}]", truncate_path(source), source.line());
    logCustomPrefix(severity, message, message_prefix);
}

void Logger::logCustomPrefix(Severity severity, const std::string& message, const std::string& prefix) 
{
    std::string severity_str;
    oof::color color;
    switch (severity) {
        case Severity::Debug: {
            severity_str = "Debug";
            color = { 200, 200, 200 };
            break;
        }
        case Severity::Info: {
            severity_str = "Info";
            color = { 79, 255, 40 };
            break;
        }
        case Severity::Warning: {
            severity_str = "Warning";
            color = { 255, 243, 0 };
            break;
        }
        case Severity::Error: {
            severity_str = "Error";
            color = { 255, 70, 70 };
            break;
        }
    }

    std::cout << oof::fg_color(color) << prefix << " " << severity_str << ": "
        << oof::reset_formatting() << message << "\n";
}

}
