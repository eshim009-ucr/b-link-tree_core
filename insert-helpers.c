#include "memory.h"
#include "insert-helpers.h"
#include <stddef.h>
#include <string.h>


//! @brief Divide by 2 and take the ceiling using only integer arithmetic
//! @param[in] x  The value to operate on
//! @return ceil(x/2)
#define DIV2CEIL(x) (((x) & 1) ? (((x)/2) + 1) : ((x)/2))


void init_node(Node *node) {
	memset(node->keys, INVALID, TREE_ORDER*sizeof(bkey_t));
}


bkey_t max(Node *node) {
	for (li_t i = TREE_ORDER-1; i > 0; --i) {
		if (node->keys[i] != INVALID) return node->keys[i];
	}
	return node->keys[0];
}


ErrorCode alloc_sibling(
	bptr_t *root, AddrNode *leaf, AddrNode *sibling
) {
	const uint_fast8_t level = get_level(leaf->addr);

	// Find an empty spot for the new leaf
	for (sibling->addr = level * MAX_NODES_PER_LEVEL;
		sibling->addr < (level+1) * MAX_NODES_PER_LEVEL;
		++sibling->addr) {
		// Found an empty slot
		if (leaf->addr != sibling->addr && mem_read(sibling->addr).keys[0] == INVALID) {
			break;
		}
	}
	sibling->node = mem_read_lock(sibling->addr);
	// If we didn't break, we didn't find an empty slot
	if (sibling->addr == (level+1) * MAX_NODES_PER_LEVEL) {
		mem_unlock(sibling->addr);
		return OUT_OF_MEMORY;
	}
	// Adjust next node pointers
	sibling->node.next = leaf->node.next;
	leaf->node.next = sibling->addr;
	// Move half of old node's contents to new node
	for (li_t i = 0; i < TREE_ORDER/2; ++i) {
		sibling->node.keys[i] = leaf->node.keys[i + (TREE_ORDER/2)];
		sibling->node.values[i] = leaf->node.values[i + (TREE_ORDER/2)];
		leaf->node.keys[i + (TREE_ORDER/2)] = INVALID;
	}

	return SUCCESS;
}


ErrorCode split_root(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	// If this is the only node
	// We need to create the first inner node
	if (is_leaf(leaf->addr)) {
		// Make a new root node
		*root = MAX_LEAVES;
	} else {
		if (*root + MAX_NODES_PER_LEVEL >= MEM_SIZE) {
			return OUT_OF_MEMORY;
		} else {
			*root = *root + MAX_NODES_PER_LEVEL;
		}
	}
	parent->addr = *root;
	parent->node = mem_read_lock(parent->addr);
	init_node(&parent->node);
	parent->node.keys[0] = leaf->node.keys[DIV2CEIL(TREE_ORDER)-1];
	parent->node.values[0].ptr = leaf->addr;
	parent->node.keys[1] = sibling->node.keys[(TREE_ORDER/2)-1];
	parent->node.values[1].ptr = sibling->addr;
	return SUCCESS;
}


ErrorCode split_nonroot(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	if (is_full(&parent->node)) {
		return PARENT_FULL;
	} else {
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			// Update key of old node
			if (parent->node.values[i].ptr == leaf->addr) {
				parent->node.keys[i] = leaf->node.keys[DIV2CEIL(TREE_ORDER)-1];
				// Scoot over other nodes to fit in new node
				for (li_t j = TREE_ORDER-1; j > i; --j) {
					parent->node.keys[j] = parent->node.keys[j-1];
					parent->node.values[j] = parent->node.values[j-1];
				}
				// Insert new node
				parent->node.keys[i+1] = sibling->node.keys[(TREE_ORDER/2)-1];
				parent->node.values[i+1].ptr = sibling->addr;
				return SUCCESS;
			}
		}
		return NOT_IMPLEMENTED;
	}
}


ErrorCode split_node(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	ErrorCode status = alloc_sibling(root, leaf, sibling);
	if (status != SUCCESS) return status;
	if (parent->addr == INVALID) {
		status = split_root(root, leaf, parent, sibling);
	} else {
		status = split_nonroot(root, leaf, parent, sibling);
	}
	if (status == SUCCESS) {
		mem_write_unlock(parent->addr, parent->node);
	}
	return status;
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
	bkey_t key, bval_t value, AddrNode *leaf, AddrNode *sibling
) {
	ErrorCode status;
	if (key < max(&leaf->node)) {
		status = insert_nonfull(&leaf->node, key, value);
	} else {
		status = insert_nonfull(&sibling->node, key, value);
	}
	mem_write_unlock(sibling->addr, sibling->node);
	mem_write_unlock(leaf->addr, leaf->node);
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
