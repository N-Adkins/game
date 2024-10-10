#pragma once

#include <vector>
#include <sol/forward.hpp>

namespace Engine {

class Event;

class EventConnection {
public:
    EventConnection(Event* event, sol::protected_function listener);
    void disconnect();
    void fireVariadic(sol::variadic_args args);

    template <typename... Args>
    void fire(Args&&... args);

private:
    Event* event;
    std::shared_ptr<sol::protected_function> listener;

    friend Event;
};

template <typename... Args>
void EventConnection::fire(Args&&... args)
{
    if (listener && listener->valid()) {
        if (auto result = listener->call(std::forward<Args>(args)...); !result.valid()) {
            sol::error err = result;
            Log::error("Lua event error at {}", err.what());
        };
    }
}

class Event {
public:
    Event() {};
    EventConnection connect(sol::protected_function listener);
    void fireVariadic(sol::variadic_args args);

    template <typename... Args>
    void fire(Args&&... args);

private:
    void removeConnection(EventConnection* connection);

    std::vector<EventConnection> connections;
    
    friend EventConnection;
};

template <typename... Args>
void Event::fire(Args&&... args)
{
    for (auto& connection : connections) {
        connection.fire(std::forward<Args>(args)...);
    }
}

} // namespace Engine
