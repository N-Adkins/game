#pragma once

#if defined(__unix__) || defined(__linux__)
#define GAME_PLATFORM_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define GAME_PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define GAME_PLATFORM_OSX
#else
#error Unsupported platform
#endif
