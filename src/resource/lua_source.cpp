#include <pch.hpp>

#include "lua_source.hpp"

namespace Engine {

LuaSource::LuaSource(const std::filesystem::path& path)
{
    Log::info("Attempting to load lua source \"{}\"", path.string());
    
    source = path;
    name = path.filename();

    Log::info("Succesfully loaded lua source \"{}\"", path.string());
}

const std::filesystem::path& LuaSource::getSource() const
{
    return source;
}

const std::string& LuaSource::getName() const
{
    return name;
}

}
