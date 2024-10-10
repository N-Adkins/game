#pragma once

#include "resource.hpp"
#include "../logging.hpp"
#include "../platform.hpp"
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Engine {

using ResourceId = size_t;

const std::filesystem::path RESOURCE_DIR = "resources";

template <typename T>
concept ResourceType = std::is_base_of_v<Resource, T> && requires (T t) {
    { T::RESOURCE_NAME } -> std::convertible_to<std::string_view>;
};

class ResourceManager {
public:
    template <ResourceType T>
    const T& load(const std::filesystem::path& path);

    template <ResourceType T>
    const T& get(const std::filesystem::path& path) const;

private:
    template <ResourceType T>
    const T* get_helper(const std::filesystem::path& path) const;

    std::unordered_map<std::string, ResourceId> path_to_id;
    std::vector<std::unique_ptr<Resource>> resources;
};

template <ResourceType T>
const T& ResourceManager::load(const std::filesystem::path& path)
{
    const ResourceId new_id = resources.size();
    path_to_id[path] = new_id;

    std::unique_ptr<T> resource = std::unique_ptr<T>(new T(getExecutablePath() / RESOURCE_DIR / path));
    const T& resource_ref = *resource.get();
    resources.push_back(std::move(resource));

    return resource_ref;
}

// Eventually here we can return a default instance if can't be found, like for a texture
// something like gmod lol, for now we just exit gracefully though

template <ResourceType T>
const T& ResourceManager::get(const std::filesystem::path& path) const
{
    if (const T* resource = get_helper<T>(path)) {
        return *resource;
    } else {
        const auto type_name = T::RESOURCE_NAME;
        Log::error("Failed to load resource of type \"{}\" at \"{}\"", type_name, path.string());
        std::exit(EXIT_FAILURE);
    }
}

template <ResourceType T>
const T* ResourceManager::get_helper(const std::filesystem::path& path) const
{
    if (!path_to_id.contains(path)) {
        return nullptr;
    }

    const ResourceId id = path_to_id.at(path);
    if (id >= resources.size()) {
        return nullptr;
    }

    const T* ptr = dynamic_cast<T*>(resources.at(id).get());
    return ptr;
}

} // namespace Engine
