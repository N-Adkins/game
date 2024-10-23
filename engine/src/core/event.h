#pragma once

/**
 * @file
 * @brief Event system
 *
 * Manages event listeners and allows sending messages to them.
 */

#include <containers/dynarray.h>
#include <defines.h>

/**
 * @brief Type of event
 */
enum event_tag {
	EVENT_TAG_WINDOW_RESIZED,

	EVENT_TAG_MAX_TAGS,
};

/**
 * @brief The data sent to an event
 *
 * There should be one payload union member for each event_tag.
 */
union event_payload {
	struct {
		i32 width;
		i32 height;
	} window_resized;
};

/**
 * @brief Alias for a callback function
 */
typedef void (*pfn_event_callback)(union event_payload payload,
				   void *user_data);

/**
 * @brief Listener (callback) for an event
 *
 * Stores an arbitrary opaque pointer so contexts can be passed to events.
 */
struct event_listener {
	pfn_event_callback callback;
	void *user_data;
};

/**
 * @brief Overall event state
 *
 * Manages dynamic lists of event_listeners, with one list for each
 * event_tag type.
 */
struct event_system {
	struct dynarray
		listeners[EVENT_TAG_MAX_TAGS]; // dynarray of struct event_listener
};

/**
 * @brief Event system initializer
 */
void event_system_startup(struct event_system *event_system);

/**
 * @brief Event system deinitializer
 */
void event_system_shutdown(struct event_system *event_system);

/**
 * @brief Registers a listener to the passed tag
 *
 * @param user_data Arbitrary pointer that will be passed back when the event
 * is fired. Should be used for context.
 */
void event_system_register(struct event_system *event_system,
			   enum event_tag tag, pfn_event_callback callback,
			   void *user_data);

/**
 * @brief Unregisters a listener from the passed tag
 *
 * This is fully based on the function pointer.
 *
 * TODO: Possibly some kind of ID system? Not sure.
 */
void event_system_unregister(struct event_system *event_system,
			     enum event_tag tag, pfn_event_callback callback);

/**
 * @briefs Fires all listeners from the passed tag
 *
 * This calls every listener under the passed tag with the passed payload,
 * and their respective user_data pointers.
 */
void event_system_fire(struct event_system *event_system, enum event_tag tag,
		       union event_payload payload);
