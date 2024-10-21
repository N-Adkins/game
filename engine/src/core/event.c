#include "event.h"

#include <core/assert.h>

void event_system_startup(struct event_system *event_system,
			  struct allocator *allocator)
{
	LASSERT(event_system != NULL);

	for (u64 i = 0; i < EVENT_TAG_MAX_TAGS; i++) {
		struct dynarray *array = &event_system->listeners[i];
		*array = dynarray_create(allocator,
					 sizeof(struct event_listener));
	}
}

void event_system_shutdown(struct event_system *event_system)
{
	LASSERT(event_system != NULL);

	for (u64 i = 0; i < EVENT_TAG_MAX_TAGS; i++) {
		struct dynarray *array = &event_system->listeners[i];
		dynarray_destroy(array);
	}
}

void event_system_register(struct event_system *event_system,
			   enum event_tag tag, pfn_event_callback callback,
			   void *user_data)
{
	LASSERT(event_system != NULL);
	LASSERT(tag < EVENT_TAG_MAX_TAGS);
	LASSERT(callback != NULL);

	const struct event_listener listener = {
		.callback = callback,
		.user_data = user_data,
	};

	struct dynarray *listeners = &event_system->listeners[tag];
	dynarray_push(listeners, listener);
}

void event_system_unregister(struct event_system *event_system,
			     enum event_tag tag, pfn_event_callback callback)
{
	LASSERT(event_system != NULL);
	LASSERT(tag < EVENT_TAG_MAX_TAGS);
	LASSERT(callback != NULL);

	struct dynarray *listeners = &event_system->listeners[tag];
	for (u64 i = 0; i < listeners->length; i++) {
		struct event_listener listener;
		const b8 result = dynarray_get(listeners, i, &listener);
		LASSERT(result);
		(void)result;

		if (listener.callback == callback) {
			dynarray_remove(listeners, i);
			break;
		}
	}
}

void event_system_fire(struct event_system *event_system, enum event_tag tag,
		       union event_payload payload)
{
	LASSERT(event_system != NULL);
	LASSERT(tag < EVENT_TAG_MAX_TAGS);

	struct dynarray *listeners = &event_system->listeners[tag];
	for (u64 i = 0; i < listeners->length; i++) {
		struct event_listener listener;
		const b8 result = dynarray_get(listeners, i, &listener);
		LASSERT(result);
		(void)result;

		listener.callback(payload, listener.user_data);
	}
}
