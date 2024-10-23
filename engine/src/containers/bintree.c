#include "bintree.h"

#include <core/assert.h>

/*
 * "Private" functions for bintree
 */
static struct bintree_node *bintree_right_rotate(struct bintree_node *node)
{
	struct bintree_node *left = node->_left;
	struct bintree_node *subtree = left->_right;
	left->_right = node;
	node->_left = subtree;
	return left;
}

static struct bintree_node *bintree_left_rotate(struct bintree_node *node)
{
	struct bintree_node *right = node->_right;
	struct bintree_node *subtree = right->_left;
	right->_left = node;
	node->_right = subtree;
	return right;
}

static u32 bintree_hash_ptr(const struct bintree_node *node)
{
	u32 hash = (u32)((u64)(uintptr_t)(node) >> 32) ^
		   (u32)((u64)(uintptr_t)(node) & 0xFFFFFFFF);
	hash ^= hash << 13;
	hash ^= hash >> 17;
	hash ^= hash << 5;
	return hash;
}

static void bintree_destroy_node(struct bintree *tree,
				 struct bintree_node *node)
{
	if (node == NULL) {
		return;
	}

	bintree_destroy_node(tree, node->_left);
	bintree_destroy_node(tree, node->_right);

	tree->free_func(node);
}

static struct bintree_node *bintree_insert_node(struct bintree *tree,
						struct bintree_node *root,
						struct bintree_node *node)
{
	if (root == NULL) {
		return node;
	}

	if (tree->compare_func(node, root) >= 0) {
		root->_left = bintree_insert_node(tree, root->_left, node);
		if (root->_left->_weight > root->_weight) {
			root = bintree_right_rotate(root);
		}
	} else {
		root->_right = bintree_insert_node(tree, root->_right, node);
		if (root->_right->_weight > root->_weight) {
			root = bintree_left_rotate(root);
		}
	}

	return root;
}

static struct bintree_node *bintree_delete_node(struct bintree *tree,
						struct bintree_node *root,
						const struct bintree_node *node)
{
	if (root == NULL) {
		return NULL;
	}

	i8 cmp = tree->compare_func(root, node);
	if (cmp == -1) {
		root->_left = bintree_delete_node(tree, root->_left, node);
	} else if (cmp == 1) {
		root->_right = bintree_delete_node(tree, root->_right, node);
	} else if (root->_left == NULL) {
		struct bintree_node *temp = root->_right;
		tree->free_func(root);
		tree->size -= 1;
		root = temp;
	} else if (root->_right == NULL) {
		struct bintree_node *temp = root->_left;
		tree->free_func(root);
		tree->size -= 1;
		root = temp;
	} else if (root->_left->_weight < root->_right->_weight) {
		root = bintree_left_rotate(root);
		root->_left = bintree_delete_node(tree, root->_left, node);
	} else {
		root = bintree_right_rotate(root);
		root->_right = bintree_delete_node(tree, root->_right, node);
	}

	return root;
}

/**
 * Actual bintree interface
 */
LAPI struct bintree bintree_create(pfn_bintree_compare compare_func,
				   pfn_bintree_free free_func)
{
	LASSERT(allocator != NULL);
	LASSERT(compare_func != NULL);
	LASSERT(free_func != NULL);

	struct bintree tree;
	tree.compare_func = compare_func;
	tree.free_func = free_func;
	tree.root = NULL;
	tree.size = 0;
	return tree;
}

LAPI void bintree_destroy(struct bintree *tree)
{
	LASSERT(tree != NULL);

	bintree_destroy_node(tree, tree->root);
}

LAPI void bintree_insert(struct bintree *tree, struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	node->_left = NULL;
	node->_right = NULL;
	node->_weight = bintree_hash_ptr(node);

	tree->root = bintree_insert_node(tree, tree->root, node);

	tree->size += 1;
}

LAPI void bintree_delete(struct bintree *tree, const struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

	tree->root = bintree_delete_node(tree, tree->root, node);
}

LAPI b8 bintree_contains(const struct bintree *tree,
			 const struct bintree_node *node)
{
	LASSERT(tree != NULL);
	LASSERT(node != NULL);

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
