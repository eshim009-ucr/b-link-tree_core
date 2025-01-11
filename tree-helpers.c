#include "tree-helpers.h"


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accommodate a tree of maximum height.
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode trace_lineage(Tree const *tree, bkey_t key, bptr_t *lineage) {
	lineage[0] = tree->root;
	li_t curr = 0;
	Node node;

	outer_loop:
	while (!is_leaf(tree, lineage[curr])) {
		node = A2S(lineage[curr]);
		// Search internal node
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			if (node.keys[i] == INVALID) {
				if (i == 0) {
					return NOT_FOUND;
				} else {
					lineage[++curr] = node.values[i-1].ptr;
					goto outer_loop;
				}
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
