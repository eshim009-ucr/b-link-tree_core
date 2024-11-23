#include <stdint.h>
#include <stdbool.h>
#include "tree.h"


inline bool is_leaf(Tree const *tree, bptr_t node_ptr) {
	// Assume leaves are stored at lowest memory addresses
	return node_ptr < MAX_LEAVES;
}

inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, MEM_SIZE*sizeof(Node));
	tree->root = 0;
}


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accomodate a tree of maximum height.
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode trace_lineage(Tree const *tree, bkey_t key, bptr_t *lineage) {
	lineage[0] = tree->root;
	li_t curr = 0;
	Node node;

	outer_loop:
	while (!is_leaf(tree, lineage[curr])) {
		node = tree->memory[lineage[curr]];
		// Search internal node
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			if (node.keys[i] == INVALID) {
				return NOT_FOUND;
			} else if (key < node.keys[i]) {
				lineage[++curr] = node.values[i].ptr;
				// Nested loops so continue doesn't work for outer loop
				goto outer_loop;
			}
		}
		// Wasn't in this node, check sibling
		if (node.next == INVALID) {
			return NOT_FOUND;
		}
		// Only keep one node per level
		lineage[curr] = node.next;
	}

	return SUCCESS;
}


ErrorCode search(Tree const *tree, bkey_t key, bval_t *value) {
	ErrorCode status;
	li_t i_leaf;
	Node leaf;
	bptr_t lineage[MAX_LEVELS];

	// Initalize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(tree, key, lineage);
	// If that failed, return the relevant error code
	if (status != SUCCESS) return status;

	for (i_leaf = MAX_LEVELS-1; i_leaf > 0; i_leaf--) {
		if (lineage[i_leaf] != INVALID) break;
	}

	// Search within the leaf node of the lineage for the key
	leaf = tree->memory[lineage[i_leaf]];
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (leaf.keys[i] == key) {
			if (value) *value = leaf.values[i];
			return SUCCESS;
		}
	}
	return NOT_FOUND;
}


ErrorCode insert(Tree *tree, bkey_t key, bval_t value) {
	return NOT_IMPLEMENTED;
}
