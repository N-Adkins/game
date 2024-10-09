#pragma once

#include <format>
#include <source_location>
#include <string>

namespace Engine::Log {

class Logger {
public:
    enum class Severity {
        Debug,
        Info,
        Warning,
        Error,
    };
    void log(Severity severity, const std::string& message, const std::source_location& source);
    void logCustomPrefix(Severity severity, const std::string& message, const std::string& prefix);
};

extern Logger GLOBAL_LOGGER;

// All of these functions have a weird thing under them that basically helps the templates
// deduce properly otherwise the source location part messes it up really bad

template <typename... Args>
struct debug {
    debug(
        std::format_string<Args...> fmt,
        Args&&... args,
        std::source_location source = std::source_location::current()
    )
    {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        GLOBAL_LOGGER.log(Logger::Severity::Debug, message, source);
    }
};
template <typename... Args>
debug(std::format_string<Args...>, Args&&...) -> debug<Args...>;

template <typename... Args>
struct info {
    info(
        std::format_string<Args...> fmt,
        Args&&... args,
        std::source_location source = std::source_location::current()
    )
    {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        GLOBAL_LOGGER.log(Logger::Severity::Info, message, source);
    }
};
template <typename... Args>
info(std::format_string<Args...>, Args&&...) -> info<Args...>;

template <typename... Args>
struct warn {
    warn(
        std::format_string<Args...> fmt,
        Args&&... args,
        std::source_location source = std::source_location::current()
    )
    {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        GLOBAL_LOGGER.log(Logger::Severity::Warning, message, source);
    }
};
template <typename... Args>
warn(std::format_string<Args...>, Args&&...) -> warn<Args...>;

template <typename... Args>
struct error {
    error(
        std::format_string<Args...> fmt,
        Args&&... args,
        std::source_location source = std::source_location::current()
    )
    {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        GLOBAL_LOGGER.log(Logger::Severity::Error, message, source);
    }
};
template <typename... Args>
error(std::format_string<Args...>, Args&&...) -> error<Args...>;

} // namespace Engine::Log
