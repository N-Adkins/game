#pragma once

#include "resource.hpp"
#include <filesystem>

namespace Engine {

class LuaSource : public Resource {
public:
    LuaSource(const std::filesystem::path& path);

    constexpr static std::string_view RESOURCE_NAME = "LuaScript";
    
    const std::filesystem::path& getSource() const;
    const std::string& getName() const;
    
private:
    std::filesystem::path source;
    std::string name;
};

} // namespace Engine
