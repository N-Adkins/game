#include <pch.hpp>

#include "event.hpp"

namespace Engine {

EventConnection::EventConnection(Event* event, sol::protected_function listener)
    : event(event), listener(std::make_shared<sol::protected_function>(listener))
{

}

void EventConnection::disconnect()
{
    event = nullptr;
    listener = nullptr;
}

void EventConnection::fireVariadic(sol::variadic_args args)
{
    if (listener && listener->valid()) {
        if (auto result = listener->call(args); !result.valid()) {
            sol::error err = result;
            Log::error("Lua event error at {}", err.what());
        };
    }
}

EventConnection Event::connect(sol::protected_function listener)
{
    connections.emplace_back(this, std::move(listener));
    return connections.back();
}

void Event::removeConnection(EventConnection* connection)
{
    auto it = std::remove_if(connections.begin(), connections.end(),
        [&connection](const EventConnection& conn) { return &conn == connection; });
    connections.erase(it, connections.end());
}

void Event::fireVariadic(sol::variadic_args args)
{
    for (auto& connection : connections) {
        connection.fire(args);
    }
}

} // namespace Engine
