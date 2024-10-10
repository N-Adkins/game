#pragma once

#include "resource.hpp"
#include <filesystem>
#include <string>

namespace Engine {

class LuaSource : public Resource {
public:
    LuaSource(const std::filesystem::path& path);

    constexpr static std::string_view RESOURCE_NAME = "LuaScript";
    
    const std::string& getSource() const;
    const std::string& getName() const;
    
private:
    std::string name;
    std::string source;
};

} // namespace Engine
