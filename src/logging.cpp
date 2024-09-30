#include "logging.hpp"
#include <cstring>
#include <format>
#include <iostream>

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
    std::string severity_str = [severity]() {
        switch (severity) {
            case Severity::Debug: return "Debug";
            case Severity::Info: return "Info";
            case Severity::Warning: return "Warning";
            case Severity::Error: return "Error";
        }
    }();
    std::string complete_message = std::format(
        "[{}:{}] {}: {}",
        truncate_path(source),
        source.line(),
        severity_str,
        message
    );
    std::cout << complete_message << "\n";
}

}
