#include "insert-helpers.h"
#include "memory.h"
#include "node.h"


bkey_t max(Node const *node) {
	for (li_t i = TREE_ORDER-1; i > 0; --i) {
		if (node->keys[i] != INVALID) return node->keys[i];
	}
	return node->keys[0];
}


ErrorCode insert_nonfull(Node *node, bkey_t key, bval_t value) {
	li_t i_insert = 0;

	for (li_t i = 0; i < TREE_ORDER; ++i) {
		// Found an empty slot
		// Will be the last slot
		if (node->keys[i] == INVALID) {
			// Scoot nodes if necessary to maintain ordering
			// Iterate right to left from the last node to the insertion point
			for (; i_insert < i; i--) {
				node->keys[i] = node->keys[i-1];
				node->values[i] = node->values[i-1];
			}
			// Do the actual insertion
			node->keys[i_insert] = key;
			node->values[i_insert] = value;
			return SUCCESS;
		} else if (node->keys[i] == key) {
			return KEY_EXISTS;
		} else if (node->keys[i] < key) {
			i_insert++;
		}
	}
	return OUT_OF_MEMORY;
}


ErrorCode insert_after_split(
	bkey_t key, bval_t value, AddrNode *leaf, AddrNode *sibling, mwrite_stream_t *mem_write_reqs
) {
	ErrorCode status;
	if (key < max(&leaf->node)) {
		status = insert_nonfull(&leaf->node, key, value);
	} else {
		status = insert_nonfull(&sibling->node, key, value);
	}
	mem_write_unlock(sibling, mem_write_reqs);
	mem_write_unlock(leaf, mem_write_reqs);
	return status;
}


ErrorCode rekey(Node *node, bkey_t old_key, bkey_t new_key) {
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == old_key) {
			node->keys[i] = new_key;
			return SUCCESS;
		}
	}
	return NOT_FOUND;
}
