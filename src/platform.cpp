#include <pch.hpp>

#include "platform.hpp"

namespace Engine {

const std::filesystem::path EXECUTABLE_PATH = [](){
#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
#elif __APPLE__
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    return std::filesystem::path(path).parent_path();
#elif __linux__
    char result[PATH_MAX];
    size_t count = static_cast<size_t>(readlink("/proc/self/exe", result, PATH_MAX));
    return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
#else
#error Unsupported platform
#endif
}();

const std::filesystem::path& getExecutablePath()
{
    return EXECUTABLE_PATH; 
}

} // namespace Engine
