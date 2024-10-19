#include "bintree.h"

#include <core/assert.h>

#define BINTREE_RED true
#define BINTREE_BLACK false

/*
 * "Private" functions for bintree
 */
static void bintree_replace_child(struct bintree *tree,
				  struct bintree_node *parent,
				  struct bintree_node *old_child,
				  struct bintree_node *new_child)
{
	LASSERT(tree != NULL);

	if (parent == NULL) {
		tree->root = new_child;
	} else if (parent->_left == old_child) {
		parent->_left = new_child;
	} else if (parent->_right == old_child) {
		parent->_right = new_child;
	} else {
		LASSERT_MSG(false, "Passed node is not a child of its parent");
	}

	if (new_child != NULL) {
		new_child->_parent = parent;
	}
}

static void bintree_rotate_right(struct bintree *tree,
				 struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	struct bintree_node *parent = node->_parent;
	struct bintree_node *left_child = node->_left;

	LASSERT(node->_left != NULL);

	node->_left = left_child->_right;
	if (left_child->_right != NULL) {
		left_child->_right->_parent = node;
	}

	left_child->_right = node;
	node->_parent = left_child;

	bintree_replace_child(tree, parent, node, left_child);
}

static void bintree_rotate_left(struct bintree *tree, struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	struct bintree_node *parent = node->_parent;
	struct bintree_node *right_child = node->_right;

	LASSERT(node->_right != NULL);

	node->_right = right_child->_left;
	if (right_child->_left != NULL) {
		right_child->_left->_parent = node;
	}

	right_child->_left = node;
	node->_parent = right_child;

	bintree_replace_child(tree, parent, node, right_child);
}

static struct bintree_node *bintree_get_uncle(struct bintree *tree,
					      struct bintree_node *parent)
{
	LASSERT(tree != NULL);
	LASSERT(parent != NULL);

	struct bintree_node *grandparent = parent->_parent;
	if (tree->compare_func(grandparent->_left, parent) == 0) {
		return grandparent->_right;
	}

	if (tree->compare_func(grandparent->_right, parent) == 0) {
		return grandparent->_left;
	}

	LASSERT_MSG(false, "Parent is not a child of its grandparent");
	return NULL; // unreachable
}

static void bintree_fix_after_insert(struct bintree *tree,
				     struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	// Case 1
	struct bintree_node *parent = node->_parent;
	if (parent == NULL) {
		node->_color = BINTREE_BLACK;
		return;
	}

	// Parent is already black, good
	if (parent->_color == BINTREE_BLACK) {
		return;
	}

	// Case 2
	struct bintree_node *grandparent = parent->_parent;
	if (grandparent == NULL) {
		parent->_color = BINTREE_BLACK;
		return;
	}

	struct bintree_node *uncle = bintree_get_uncle(tree, parent);

	// Case 3
	if (uncle != NULL && uncle->_color == BINTREE_RED) {
		parent->_color = BINTREE_BLACK;
		grandparent->_color = BINTREE_RED;
		uncle->_color = BINTREE_BLACK;

		bintree_fix_after_insert(tree, grandparent);
	} else if (parent == grandparent->_left) {
		// Case 4a
		if (node == parent->_right) {
			bintree_rotate_left(tree, parent);
			parent = node;
		}

		// Case 5a
		bintree_rotate_right(tree, grandparent);

		parent->_color = BINTREE_BLACK;
		grandparent->_color = BINTREE_RED;
	} else {
		// Case 4b
		if (node == parent->_left) {
			bintree_rotate_right(tree, parent);
			parent = node;
		}

		// Case 5b
		bintree_rotate_left(tree, grandparent);

		parent->_color = BINTREE_BLACK;
		grandparent->_color = BINTREE_RED;
	}
}

struct bintree_node *bintree_delete_zero_or_one_child(struct bintree *tree,
						      struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	if (node->_left != NULL) {
		bintree_replace_child(tree, node->_parent, node, node->_left);
		return node->_left;
	}

	if (node->_right != NULL) {
		bintree_replace_child(tree, node->_parent, node, node->_right);
		return node->_right;
	}

	if (node->_color == BINTREE_BLACK) {
		node->_nil = true;
	} else {
		tree->free_func(tree->allocator, node);
		return NULL;
	}

	return node;
}

static struct bintree_node *bintree_find_min(struct bintree_node *node)
{
	LASSERT(node != NULL);

	while (node->_left != NULL) {
		node = node->_left;
	}
	return node;
}

static struct bintree_node *bintree_get_sibling(struct bintree_node *node)
{
	LASSERT(node != NULL);

	struct bintree_node *parent = node->_parent;

	if (node == parent->_left) {
		return parent->_right;
	}

	if (node == parent->_right) {
		return parent->_left;
	}

	LASSERT("Parent is not a child of its grandparent");
	return NULL; // unreachable
}

static b8 bintree_is_black(struct bintree_node *node)
{
	return node == NULL || node->_color == BINTREE_BLACK;
}

static void bintree_handle_red_sibling(struct bintree *tree,
				       struct bintree_node *node,
				       struct bintree_node *sibling)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);
	LASSERT(sibling != NULL);

	sibling->_color = BINTREE_BLACK;
	node->_parent->_color = BINTREE_RED;

	if (node == node->_parent->_left) {
		bintree_rotate_left(tree, node->_parent);
	} else {
		bintree_rotate_right(tree, node->_parent);
	}
}

static void bintree_handle_black_sibling_red_child(struct bintree *tree,
						   struct bintree_node *node,
						   struct bintree_node *sibling)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);
	LASSERT(sibling != NULL);

	const b8 node_is_left_child = node == node->_parent->_left;

	// Case 5
	if (node_is_left_child && bintree_is_black(sibling->_right)) {
		sibling->_left->_color = BINTREE_BLACK;
		sibling->_color = BINTREE_RED;
		bintree_rotate_right(tree, sibling);
		sibling = node->_parent->_right;
	} else if (!node_is_left_child && bintree_is_black(sibling->_left)) {
		sibling->_right->_color = BINTREE_BLACK;
		sibling->_color = BINTREE_RED;
		bintree_rotate_left(tree, sibling);
		sibling = node->_parent->_left;
	}

	// Case 6
	sibling->_color = node->_parent->_color;
	node->_parent->_color = BINTREE_BLACK;
	if (node_is_left_child) {
		sibling->_right->_color = BINTREE_BLACK;
		bintree_rotate_left(tree, node->_parent);
	} else {
		sibling->_left->_color = BINTREE_BLACK;
		bintree_rotate_right(tree, node->_parent);
	}
}

static void bintree_fix_after_delete(struct bintree *tree,
				     struct bintree_node *node)
{
	// Case 1
	if (node == tree->root) {
		node->_color = BINTREE_BLACK;
		return;
	}

	struct bintree_node *sibling = bintree_get_sibling(node);

	// Case 2
	if (sibling->_color == BINTREE_RED) {
		bintree_handle_red_sibling(tree, node, sibling);
		sibling = bintree_get_sibling(node);
	}

	// Cases 3 and 4
	if (bintree_is_black(sibling->_left) &&
	    bintree_is_black(sibling->_right)) {
		sibling->_color = BINTREE_RED;

		if (node->_parent->_color == BINTREE_RED) {
			node->_parent->_color = BINTREE_BLACK;
		} else {
			bintree_fix_after_delete(tree, node->_parent);
		}
	} else { // Cases 5 and 6
		bintree_handle_black_sibling_red_child(tree, node, sibling);
	}
}

void bintree_destroy_node(struct bintree *tree, struct bintree_node *node)
{
    if (node == NULL) {
        return;
    }

    bintree_destroy_node(tree, node->_left);
    bintree_destroy_node(tree, node->_right);

    tree->free_func(tree->allocator, node);
}

/**
 * Actual bintree interface
 */

LAPI struct bintree bintree_create(struct allocator *allocator,
				   pfn_bintree_compare compare_func,
				   pfn_bintree_free free_func)
{
	LASSERT(allocator != NULL);
	LASSERT(compare_func != NULL);
	LASSERT(free_func != NULL);

	struct bintree tree;
	tree.allocator = allocator;
	tree.compare_func = compare_func;
	tree.free_func = free_func;
	tree.root = NULL;
	return tree;
}

LAPI void bintree_destroy(struct bintree *tree)
{
	bintree_destroy_node(tree, tree->root);
}

LAPI void bintree_insert(struct bintree *tree, struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	struct bintree_node *iter = tree->root;
	struct bintree_node *parent = NULL;
	i8 cmp = 0;

	while (iter != NULL) {
		parent = iter;

		cmp = tree->compare_func(iter, node);
		if (cmp == -1) {
			iter = iter->_left;
		} else if (cmp == 1) {
			iter = iter->_right;
		} else {
			LERROR("Attempted to insert value into bintree that already exists");
			tree->free_func(tree->allocator, node);
			return;
		}
	}

	node->_color = BINTREE_RED;
	node->_left = NULL;
	node->_right = NULL;
	node->_nil = false;
	if (parent == NULL) {
		tree->root = node;
	} else if (cmp == -1) {
		parent->_left = node;
	} else {
		parent->_right = node;
	}
	node->_parent = parent;

	bintree_fix_after_insert(tree, node);
}

LAPI void bintree_delete(struct bintree *tree, const struct bintree_node *node)
{
	struct bintree_node *iter = tree->root;
	i8 cmp = 0;

	while (iter != NULL && (cmp = tree->compare_func(iter, node)) != 0) {
		if (cmp == -1) {
			iter = iter->_left;
		} else {
			iter = iter->_right;
		}
	}

	// Doesn't exist
	if (iter == NULL) {
		LWARN("Attempted to delete node that doesn't exist");
		return;
	}

	struct bintree_node *moved_up = NULL;
	b8 deleted_color = BINTREE_BLACK;

	if (iter->_left == NULL || iter->_right == NULL) {
		moved_up = bintree_delete_zero_or_one_child(tree, iter);
		deleted_color = iter->_color;
	}

	else {
		struct bintree_node *inorder_successor =
			bintree_find_min(iter->_right);
		struct bintree_node old_successor = *inorder_successor;
		*inorder_successor = *iter;
		deleted_color = iter->_color;
		*iter = old_successor;
		moved_up = bintree_delete_zero_or_one_child(tree, iter);
	}

	if (deleted_color == BINTREE_BLACK) {
		bintree_fix_after_delete(tree, moved_up);

		if (moved_up->_nil) {
			tree->free_func(tree->allocator, moved_up);
			bintree_replace_child(tree, moved_up->_parent, moved_up,
					      NULL);
		}
	}
}

LAPI b8 bintree_contains(struct bintree *tree, const struct bintree_node *node)
{
	struct bintree_node *iter = tree->root;

	while (iter != NULL) {
		const i8 cmp = tree->compare_func(iter, node);
		if (cmp == -1) {
			iter = iter->_left;
		} else if (cmp == 1) {
			iter = iter->_right;
		} else { // equal
			return true;
		}
	}

	return false;
}
