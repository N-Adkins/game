#pragma once

#include "../constructors.hpp"

namespace Engine {

class Resource {
public:
    Resource() = default;
    virtual ~Resource() = default;
    DELETE_COPY(Resource);
    DEFAULT_MOVE(Resource);
};

} // namespace Engine
