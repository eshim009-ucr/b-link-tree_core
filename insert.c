#include "insert.h"
#include "tree-helpers.h"
#include "memory.h"
#include "insert-helpers.h"
#include <string.h>


ErrorCode insert(bptr_t *root, bkey_t key, bval_t value) {
	ErrorCode status;
	li_t i_leaf;
	AddrNode leaf, parent, sibling;
	bptr_t lineage[MAX_LEVELS];
	bool keep_splitting = false;

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(*root, key, lineage);
	if (status != SUCCESS) return status;
	// Load leaf
	i_leaf = get_leaf_idx(lineage);
	leaf.addr = lineage[i_leaf];
	leaf.node = mem_read_lock(leaf.addr);
	do {
		// Load this node's parent, if it exists
		if (i_leaf > 0) {
			parent.addr = lineage[i_leaf-1];
			parent.node = mem_read_lock(parent.addr);
		} else {
			parent.addr = INVALID;
		}

		if (!is_full(&leaf.node)) {
			status = insert_nonfull(&leaf.node, key, value);
			mem_write_unlock(&leaf);
			if (parent.addr != INVALID) mem_unlock(parent.addr);
			if (status != SUCCESS) return status;
		} else {
			// Try to split this node
			status = split_node(root, &leaf, &parent, &sibling);
			keep_splitting = (status == PARENT_FULL);
			// Unrecoverable failure
			if (status != SUCCESS && status != PARENT_FULL) {
				mem_unlock(leaf.addr);
				mem_unlock(sibling.addr);
				mem_unlock(parent.addr);
				return status;
			}
			// Insert the new content and unlock leaf and its sibling
			status = insert_after_split(key, value, &leaf, &sibling);
			if (keep_splitting) {
				// Try this again on the parent
				key = max(&sibling.node);
				rekey(&parent.node, key, max(&leaf.node));
				value.ptr = sibling.addr;
				i_leaf--;
				leaf = parent;
			} else if (status != SUCCESS) {
				mem_unlock(parent.addr);
				return status;
			}
		}
	} while (keep_splitting);

	return SUCCESS;
}
