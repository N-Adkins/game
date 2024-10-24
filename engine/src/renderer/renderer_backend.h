#pragma once

#include <defines.h>

/**
 * @brief Types of supported renderer backends
 */
enum renderer_backend_type {
	RENDERER_BACKEND_VULKAN,
};

/**
 * @brief Abstraction layer over the renderer backends
 * Uses a vtable and an opaque handle to make an interface - this should ideally
 * only be used by the renderer frontend.
 */
struct renderer_backend {
	struct {
		LRESULT (*startup)
		(struct renderer_backend *backend, const char *app_name);
		void (*shutdown)(struct renderer_backend *backend);
		void (*window_resize)(struct renderer_backend *backend,
				      i32 width, i32 height);
		void (*begin_frame)(struct renderer_backend *backend,
				    f32 delta_time);
		void (*end_frame)(struct renderer_backend *backend,
				  f32 delta_time);
	} vtable;
	void *impl;
};

/**
 * This function does two things - it allocates the respective backend's state and populates the
 * vtable, it then calls the underlying startup function and returns the result of that.
 */
LRESULT renderer_backend_startup(enum renderer_backend_type type,
				 const char *app_name,
				 struct renderer_backend *backend);

/**
 * @brief Deinitializes the specified renderer_backend
 */
void renderer_backend_shutdown(struct renderer_backend *backend);

/**
 * @brief 
 */
void renderer_backend_window_resize(struct renderer_backend *backend, i32 width,
				    i32 height);
void renderer_backend_begin_frame(struct renderer_backend *backend,
				  f32 delta_time);
void renderer_backend_end_frame(struct renderer_backend *backend,
				f32 delta_time);
