#include "tree-helpers.h"
#include "memory.h"
#include "node.h"


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accommodate a tree of maximum height.
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode trace_lineage(bptr_t root, bkey_t key, bptr_t *lineage) {
	lineage[0] = root;
	li_t curr = 0;
	Node node;

	// Iterate until we hit a leaf
	outer_loop:
	while (!is_leaf(lineage[curr])) {
		node = mem_read(lineage[curr]);
		// Search internal node
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			// We overshot the node we were looking for
			// and got an uninitialized key
			if (node.keys[i] == INVALID) {
				// Empty node, error
				if (i == 0) {
					return NOT_FOUND;
				}
				// Save the last node we looked at
				else {
					lineage[++curr] = node.values[i-1].ptr;
					goto outer_loop;
				}
			}
			// If this key is the first key greater than what we're looking for
			// then continue down this subtree
			else if (key <= node.keys[i]) {
				lineage[++curr] = node.values[i].ptr;
				// Nested loops so continue doesn't work for outer loop
				goto outer_loop;
			}
		}
		// Wasn't in this node, check sibling
		if (node.next == INVALID) {
			// Got to the farthest right child,
			// so the key is greater than any current tree value
			lineage[++curr] = node.values[TREE_ORDER-1].ptr;
			continue;
		}
		// Only keep one node per level, so don't increment the height index
		lineage[curr] = node.next;
	}

	return SUCCESS;
}
