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

namespace Engine {

const std::filesystem::path& getExecutablePath();

} // namespace Engine
