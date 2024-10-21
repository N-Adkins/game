#pragma once

#include <containers/dynarray.h>
#include <defines.h>

enum event_tag {
	EVENT_TAG_WINDOW_RESIZED,

	EVENT_TAG_MAX_TAGS,
};

union event_payload {
	struct {
		i32 width;
		i32 height;
	} window_resized;
};

typedef void (*pfn_event_callback)(union event_payload payload,
				   void *user_data);

struct event_listener {
	pfn_event_callback callback;
	void *user_data;
};

struct event_system {
	struct dynarray
		listeners[EVENT_TAG_MAX_TAGS]; // dynarray of struct event_listener
};

void event_system_startup(struct event_system *event_system,
			  struct allocator *allocator);
void event_system_shutdown(struct event_system *event_system);
void event_system_register(struct event_system *event_system,
			   enum event_tag tag, pfn_event_callback callback,
			   void *user_data);
void event_system_unregister(struct event_system *event_system,
			     enum event_tag tag, pfn_event_callback callback);
void event_system_fire(struct event_system *event_system, enum event_tag tag,
		       union event_payload payload);
