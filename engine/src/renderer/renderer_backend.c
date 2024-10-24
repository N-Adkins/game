#include "renderer_backend.h"
#include "renderer/vulkan/vulkan_backend.h"

LRESULT renderer_backend_startup(enum renderer_backend_type type,
				 const char *app_name,
				 struct renderer_backend *backend)
{
	switch (type) {
	case RENDERER_BACKEND_VULKAN:
		*backend = create_vulkan_backend();
		break;
	}

	return backend->vtable.startup(backend, app_name);
}

void renderer_backend_shutdown(struct renderer_backend *backend)
{
	backend->vtable.shutdown(backend);
}

void renderer_backend_window_resize(struct renderer_backend *backend, i32 width,
				    i32 height)
{
	backend->vtable.window_resize(backend, width, height);
}

void renderer_backend_begin_frame(struct renderer_backend *backend,
				  f32 delta_time)
{
	backend->vtable.begin_frame(backend, delta_time);
}

void renderer_backend_end_frame(struct renderer_backend *backend,
				f32 delta_time)
{
	backend->vtable.end_frame(backend, delta_time);
}
