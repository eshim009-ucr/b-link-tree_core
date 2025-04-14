#include "insert.h"
#include "insert-helpers.h"
#include "memory.h"
#include "node.h"
#include "split.h"
#include "tree-helpers.h"
#include <string.h>


ErrorCode insert(bptr_t *root, bkey_t key, bval_t value, mread_req_stream_t *mem_read_reqs, mread_resp_stream_t *mem_read_resps, mwrite_stream_t *mem_write_reqs) {
	ErrorCode status;
	li_t i_leaf;
	AddrNode leaf, parent, sibling;
	bptr_t lineage[MAX_LEVELS];
	bool keep_splitting = false;

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(*root, key, lineage, mem_read_reqs, mem_read_resps);
	if (status != SUCCESS) return status;
	// Load leaf
	i_leaf = get_leaf_idx(lineage);
	leaf.addr = lineage[i_leaf];
	leaf.node = mem_read_lock(leaf.addr, mem_read_reqs, mem_read_resps);
	do {
		// Load this node's parent, if it exists
		if (i_leaf > 0) {
			parent.addr = lineage[i_leaf-1];
			parent.node = mem_read_lock(parent.addr, mem_read_reqs, mem_read_resps);
		} else {
			parent.addr = INVALID;
		}

		if (!is_full(&leaf.node)) {
			status = insert_nonfull(&leaf.node, key, value);
			mem_write_unlock(&leaf, mem_write_reqs);
			if (parent.addr != INVALID) mem_write_unlock(&parent, mem_write_reqs);
			if (status != SUCCESS) return status;
		} else {
			// Try to split this node
			status = split_node(root, &leaf, &parent, &sibling, mem_read_reqs, mem_read_resps, mem_write_reqs);
			keep_splitting = (status == PARENT_FULL);
			// Unrecoverable failure
			if (status != SUCCESS && status != PARENT_FULL) {
				mem_write_unlock(&leaf, mem_write_reqs);
				mem_write_unlock(&sibling, mem_write_reqs);
				mem_write_unlock(&parent, mem_write_reqs);
				return status;
			}
			// Insert the new content and unlock leaf and its sibling
			status = insert_after_split(key, value, &leaf, &sibling, mem_write_reqs);
			if (keep_splitting) {
				// Try this again on the parent
				key = max(&sibling.node);
				rekey(&parent.node, key, max(&leaf.node));
				value.ptr = sibling.addr;
				i_leaf--;
				leaf = parent;
			} else if (status != SUCCESS) {
				mem_write_unlock(&parent, mem_write_reqs);
				return status;
			}
		}
	} while (keep_splitting);

	return SUCCESS;
}
