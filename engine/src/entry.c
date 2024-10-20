#include "entry.h"

#include <core/assert.h>
#include <core/memory.h>
#include <core/logger.h>
#include <containers/bintree.h>
#include <containers/dynarray.h>
#include <platform/platform.h>
#include <defines.h>

struct int_node {
	int value;
	struct bintree_node node;
};

i8 int_compare(const struct bintree_node *left,
	       const struct bintree_node *right)
{
	const struct int_node *left_int =
		LCONTAINER_OF(left, const struct int_node, node);
	const struct int_node *right_int =
		LCONTAINER_OF(right, const struct int_node, node);
	if (left_int->value > right_int->value) {
		return -1;
	}
	if (left_int->value < right_int->value) {
		return 1;
	}
	return 0;
}

void int_free(struct allocator *allocator, struct bintree_node *node)
{
	struct int_node *int_node = LCONTAINER_OF(node, struct int_node, node);
	allocator_free(allocator, int_node, sizeof(struct int_node),
		       MEMORY_TAG_ARRAY);
}

LAPI int real_main(void)
{
	LINFO("Compiler: %s, Platform: %s", LCOMPILER, LPLATFORM);

	const i32 START_X = 640;
	const i32 START_Y = 360;
	const i32 START_WIDTH = 1280;
	const i32 START_HEIGHT = 720;
	struct platform platform;
	platform_startup(&platform, "App", START_X, START_Y, START_WIDTH,
			 START_HEIGHT);

	struct allocator alloc = allocator_create();
	int *ptr = allocator_alloc(&alloc, sizeof(int), MEMORY_TAG_UNKNOWN);
	allocator_free(&alloc, ptr, sizeof(int), MEMORY_TAG_UNKNOWN);

	struct dynarray array = dynarray_create(&alloc, sizeof(int));
	dynarray_push(&array, 120312);
	dynarray_get(&array, 1, ptr);
	dynarray_destroy(&array);

	struct bintree tree = bintree_create(&alloc, int_compare, int_free);
	for (int i = 3282; i > 0; i--) {
		struct int_node *node = allocator_alloc(
			&alloc, sizeof(struct int_node), MEMORY_TAG_ARRAY);
		node->value = i;
		bintree_insert(&tree, &node->node);
	}
	struct int_node dummy_node = {
		.value = 2812,
	};
	LINFO("Contains 2812: %d", bintree_contains(&tree, &dummy_node.node));
	bintree_delete(&tree, &dummy_node.node);
	LINFO("Contains 2812: %d", bintree_contains(&tree, &dummy_node.node));
	bintree_destroy(&tree);

	// Testing some mutex error stuff
	struct mutex mutex = mutex_create();
	mutex_lock(mutex);
	mutex_lock(mutex);
	mutex_unlock(mutex);
	mutex_unlock(mutex);
	mutex_destroy(mutex);

	while (platform_poll_events(platform)) {
	}

	allocator_destroy(&alloc);
	platform_shutdown(platform);

	return 0;
}
