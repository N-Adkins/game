#pragma once

/**
 * @file
 * @brief Binary search tree container
 *
 * A general-use binary search tree. The following operations are guaranteed to be 
 * average-case O(logn):
 * - Insertion
 * - Deletion
 * - Searching
 *
 * The actual implementation of the tree is fully subject to change, meaning it could be 
 * a red-black tree (most likely in the future), a treap, or something else that 
 * guarantees those time complexity constraints.
 */

#include <core/memory.h>
#include <defines.h>

/**
 * @brief The underlying binary search tree node type
 *
 * This has no guaranteed layout, don't access any members within
 * this. It can't be opaque because that would make it impossible
 * to use in a generic manner.
 */
struct bintree_node {
	struct bintree_node *_left;
	struct bintree_node *_right;
	u32 _weight;
};

/**
 * @brief Comparison function for binary tree nodes
 *
 * -1 means left was more than right
 *  0 means they were equal
 *  1 means right was more than left
 */
typedef i8 (*pfn_bintree_compare)(const struct bintree_node *left,
				  const struct bintree_node *right);

/**
 * @brief Destruction function for binary tree node
 *
 * This function should completely free the passed node, including the pointer to the parent
 * struct that is wrapping the bintree_node.
 */
typedef void (*pfn_bintree_free)(struct allocator *allocator,
				 struct bintree_node *node);

/** 
 * @brief Binary search tree
 *
 * This has no set implementation, so don't depend on one. Currently it
 * is a treap. The usage of this struct relies on LCONTAINER_OF. To make 
 * a node type, you need to embed the bintree_node type into the desired 
 * node type.
 *
 * Example:
 *
 * struct int_node {
 *     struct bintree_node node;
 *     int x;
 * };
 */
struct bintree {
	struct allocator *allocator;
	pfn_bintree_compare compare_func;
	pfn_bintree_free free_func;
	struct bintree_node *root;
	u64 size;
};

/**
 * @brief Creates binary search tree
 *
 * This function does not allocate memory.
 */
LAPI struct bintree bintree_create(struct allocator *allocator,
				   pfn_bintree_compare compare_func,
				   pfn_bintree_free free_func);

/**
 * @brief Destroys binary search tree
 */
LAPI void bintree_destroy(struct bintree *tree);

/**
 * @brief Inserts the passed node into the binary search tree
 *
 * Note that the passed pointer is directly used in the tree, hence
 * no const qualification.
 *
 * Guaranteed average case O(logn)
 */
LAPI void bintree_insert(struct bintree *tree, struct bintree_node *node);

/**
 * @brief Deletes the passed node's equivalent from the binary search tree
 *
 * This node is not inserted into the tree in any way, it's just the only
 * way to generically check nodes using the check_func.
 *
 * Guaranteed average case O(logn)
 */
LAPI void bintree_delete(struct bintree *tree, const struct bintree_node *node);

/**
 * @brief Checks if bintree contains the passed node's equivalent
 *
 * Guaranteed average case O(logn)
 *
 * @returns true if the tree has the value, false if it doesn't
 */
LAPI b8 bintree_contains(const struct bintree *tree,
			 const struct bintree_node *node);
