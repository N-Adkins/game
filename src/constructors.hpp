#ifndef GAME_CONSTRUCTORS_HPP
#define GAME_CONSTRUCTORS_HPP

#define DELETE_COPY(name)                               \
    name(const name&) = delete;                         \
    name& operator=(const name&) = delete

#define DEFAULT_COPY(name)                              \
    name(const name&) = default;                        \
    name& operator=(const name&) = default

#define DELETE_MOVE(name)                               \
    name(name&&) = delete;                              \
    name& operator=(name&&) = delete

#define DEFAULT_MOVE(name)                              \
    name(name&&) = default;                             \
    name& operator=(name&&) = default

#define DELETE_MOVE_COPY(name)                          \
    DELETE_COPY(name)                                   \
    DELETE_MOVE(name)

#endif
