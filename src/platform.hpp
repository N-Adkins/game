#pragma once

#include <filesystem>

#if defined(__unix__) || defined(__linux__)
#define GAME_PLATFORM_LINUX
#include <unistd.h>
#include <linux/limits.h>
#elif defined(_WIN32) || defined(_WIN64)
#define GAME_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(__APPLE__)
#define GAME_PLATFORM_OSX
#include <mach-o/dyld.h>
#else
#error Unsupported platform
#endif

#if defined(__clang__)
#define GAME_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define GAME_COMPILER_GCC
#elif defined(_MSC_VER)
#define GAME_COMPILER_MSVC
#else
#error Unsupported compiler
#endif

#if defined(GAME_COMPILER_CLANG) || defined(GAME_COMPILER_GCC)
#define GAME_PACKED_STRUCT(name, ...) \
struct name __VA_ARGS__ __attribute__((__packed__));
#define GAME_PACKED_CLASS(name, ...) \
class name __VA_ARGS__ __attribute__((__packed__));
#else
#define GAME_PACKED_STRUCT(name, ...) \
#pragma pack(push, 1) \
struct name __VA_ARGS__; \
#pragma pack(pop) \
#define GAME_PACKED_CLASS(name, ...) \
#pragma pack(push, 1) \
class name __VA_ARGS__; \
#pragma pack(pop)
#endif

namespace Engine {

const std::filesystem::path& getExecutablePath();

} // namespace Engine
