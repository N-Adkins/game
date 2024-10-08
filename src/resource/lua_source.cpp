#include <pch.hpp>

#include "lua_source.hpp"
#include <fstream>

namespace Engine {

LuaSource::LuaSource(const std::filesystem::path& path)
{
    Log::info("Attempting to load lua source \"{}\"", path.string());

    std::ifstream file(path);
    if (!file) {
        Log::error("Lua source \"{}\" could not be found or opened", path.string());
        return;
    }
    
    std::stringstream buf;
    buf << file.rdbuf();

    Log::info("Succesfully loaded lua source \"{}\"", path.string());

    source = buf.str();
    name = path.filename();
}

const std::string& LuaSource::getSource() const
{
    return source;
}

const std::string& LuaSource::getName() const
{
    return name;
}

}
