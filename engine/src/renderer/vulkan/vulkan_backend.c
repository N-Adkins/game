#include "vulkan_backend.h"

#include <platform/platform.h>
#include <renderer/renderer_backend.h>
#include <containers/dynarray.h>
#include <core/assert.h>
#include <core/memory.h>
#include <utils/string.h>
#include <defines.h>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

struct vulkan_backend {
	VkInstance instance;
};

static const b8 ENABLE_VALIDATION_LAYERS =
#ifdef LENGINE_DEBUG
	true
#else
	false
#endif
	;

/**
 * Requested validation layer string names
 */
static const char *const VALIDATION_LAYERS[] = {
	"VK_LAYER_KHRONOS_validation",
};

/**
 * Placeholder for when we make an actual allocator.
 */
static void *vulkan_allocator = NULL;

/**
 * @brief Checks for vulkan call errors safely
 *
 * If the passed call fails, it prints the entire call to the error log.
 * Doesn't propogate any result code.
 */
#define VK_CALL(vk_call)                                                  \
	do {                                                              \
		const LTYPEOF(vk_call) LUNIQUE_ID(0) = vk_call;           \
		if (LUNIQUE_ID(0) != VK_SUCCESS) {                        \
			LERROR("Vulkan call failed with %s : %s",         \
			       string_VkResult(LUNIQUE_ID(0)), #vk_call); \
		}                                                         \
	} while (0)

/**
 * @brief Checks for vulkan call errors and returns an error it does
 *
 * If the passed call fails, it prints the entire call to the error log
 * and returns 0.
 */
#define VK_CALL_RETURN(vk_call)                                           \
	do {                                                              \
		const LTYPEOF(vk_call) LUNIQUE_ID(0) = vk_call;           \
		if (LUNIQUE_ID(0) != VK_SUCCESS) {                        \
			LERROR("Vulkan call failed with %s : %s",         \
			       string_VkResult(LUNIQUE_ID(0)), #vk_call); \
			return LRESULT_FAILURE;                           \
		}                                                         \
	} while (0)

static b8 vulkan_check_validation_layer_support(void)
{
	u32 layer_count = 0;
	VK_CALL(vkEnumerateInstanceLayerProperties(&layer_count, NULL));

	struct dynarray available_layers =
		dynarray_create_length(layer_count, sizeof(VkLayerProperties));
	VK_CALL(vkEnumerateInstanceLayerProperties(&layer_count,
						   available_layers.values));

	for (u64 i = 0; i < LARRAY_LENGTH(VALIDATION_LAYERS); i++) {
		b8 layer_found = false;
		const char *required_layer = VALIDATION_LAYERS[i];

		for (u64 j = 0; j < layer_count; j++) {
			VkLayerProperties available_layer = {};
			dynarray_get(&available_layers, j, &available_layer);
			if (strings_equal(required_layer,
					  available_layer.layerName)) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) {
			dynarray_destroy(&available_layers);
			return false;
		}
	}

	dynarray_destroy(&available_layers);
	return true;
}

LRESULT vulkan_backend_startup(struct renderer_backend *renderer_backend,
			       const char *app_name)
{
	LASSERT(renderer_backend != NULL);
	LASSERT(app_name != NULL);

	struct vulkan_backend *vulkan = renderer_backend->impl;

	if (ENABLE_VALIDATION_LAYERS) {
		LINFO("Validation layers enabled, checking support");
		if (!vulkan_check_validation_layer_support()) {
			LERROR("Failed to find all requested validation layers");
			return LRESULT_FAILURE;
		}
	}

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = app_name,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "Less Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};

	struct dynarray required_extensions =
		dynarray_create(sizeof(const char **));
	dynarray_push(&required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);
	platform_get_required_extensions(&required_extensions);

	VkInstanceCreateInfo instance_create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.ppEnabledExtensionNames =
			(const char **)required_extensions.values,
		.enabledExtensionCount = required_extensions.length,
	};

	if (ENABLE_VALIDATION_LAYERS) {
		instance_create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
		instance_create_info.enabledLayerCount =
			LARRAY_LENGTH(VALIDATION_LAYERS);
	} else {
		instance_create_info.ppEnabledLayerNames = NULL;
		instance_create_info.enabledLayerCount = 0;
	}

	VkInstance instance = {};
	VK_CALL(vkCreateInstance(&instance_create_info, vulkan_allocator,
				 &instance));
	dynarray_destroy(&required_extensions);

	*vulkan = (struct vulkan_backend){
		.instance = instance,
	};

	return LRESULT_SUCCESS;
}

#undef VK_CHECK

void vulkan_backend_shutdown(struct renderer_backend *renderer_backend)
{
	LASSERT(renderer_backend != NULL);

	struct vulkan_backend *vulkan = renderer_backend->impl;

	vkDestroyInstance(vulkan->instance, vulkan_allocator);

	engine_free(renderer_backend->impl, sizeof(struct vulkan_backend),
		    MEMORY_TAG_VULKAN);
}

void vulkan_backend_window_resize(struct renderer_backend *renderer_backend,
				  i32 width, i32 height)
{
	LASSERT(renderer_backend != NULL);
	(void)renderer_backend;
	(void)width;
	(void)height;
}

void vulkan_backend_begin_frame(struct renderer_backend *renderer_backend,
				f32 delta_time)
{
	LASSERT(renderer_backend != NULL);
	(void)renderer_backend;
	(void)delta_time;
}

void vulkan_backend_end_frame(struct renderer_backend *renderer_backend,
			      f32 delta_time)
{
	LASSERT(renderer_backend != NULL);
	(void)renderer_backend;
	(void)delta_time;
}

struct renderer_backend create_vulkan_backend()
{
	struct renderer_backend renderer_backend;

	struct vulkan_backend *vulkan_backend =
		engine_alloc(sizeof(struct vulkan_backend), MEMORY_TAG_VULKAN);
	renderer_backend.impl = vulkan_backend;

	renderer_backend.vtable.startup = &vulkan_backend_startup;
	renderer_backend.vtable.shutdown = &vulkan_backend_shutdown;
	renderer_backend.vtable.window_resize = &vulkan_backend_window_resize;
	renderer_backend.vtable.begin_frame = &vulkan_backend_begin_frame;
	renderer_backend.vtable.end_frame = &vulkan_backend_end_frame;

	return renderer_backend;
}
