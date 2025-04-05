#include "insert.h"
#include "tree-helpers.h"
#include "memory.h"
#include "node.h"
#include <stddef.h>
#include <string.h>


//! @brief Divide by 2 and take the ceiling using only integer arithmetic
//! @param[in] x  The value to operate on
//! @return ceil(x/2)
#define DIV2CEIL(x) (((x) & 1) ? (((x)/2) + 1) : ((x)/2))


//! @brief Clear a node's keys
//! @param[in] node  The node whose keys should be cleared
inline static void init_node(Node *node) {
	memset(node->keys, INVALID, TREE_ORDER*sizeof(bkey_t));
}

//! @brief Find the maximum key in a node
//! @param[in] node  The node to check
//! @return The largest valid key in the node
inline static bkey_t max(Node *node) {
	for (li_t i = TREE_ORDER-1; i > 0; --i) {
		if (node->keys[i] != INVALID) return node->keys[i];
	}
	return node->keys[0];
}


//! @brief Allocate a new sibling node in an empty slot in main mameory
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf_addr     The address of the node to split
//! @param[in]     leaf          The node to split
//! @param[inout]  parent_addr   The address of the parent of the node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling_addr  The address of the split node's new sibling
//! @param[out]    sibling       The contents of the split node's new sibling
//!
//! Acquires a lock on the sibling node
static ErrorCode alloc_sibling(bptr_t *root,
	bptr_t leaf_addr, Node *leaf,
	bptr_t *sibling_addr, Node *sibling) {
	const uint_fast8_t level = get_level(leaf_addr);

	// Find an empty spot for the new leaf
	for (*sibling_addr = level * MAX_NODES_PER_LEVEL;
		*sibling_addr < (level+1) * MAX_NODES_PER_LEVEL;
		++(*sibling_addr)) {
		// Found an empty slot
		if (leaf_addr != *sibling_addr && mem_read(*sibling_addr).keys[0] == INVALID) {
			break;
		}
	}
	*sibling = mem_read_lock(*sibling_addr);
	// If we didn't break, we didn't find an empty slot
	if (*sibling_addr == (level+1) * MAX_NODES_PER_LEVEL) {
		mem_unlock(*sibling_addr);
		return OUT_OF_MEMORY;
	}
	// Adjust next node pointers
	sibling->next = leaf->next;
	leaf->next = *sibling_addr;
	// Move half of old node's contents to new node
	for (li_t i = 0; i < TREE_ORDER/2; ++i) {
		sibling->keys[i] = leaf->keys[i + (TREE_ORDER/2)];
		sibling->values[i] = leaf->values[i + (TREE_ORDER/2)];
		leaf->keys[i + (TREE_ORDER/2)] = INVALID;
	}

	return SUCCESS;
}


//! @brief Assign an allocated sibling pair at the root leve of the tree
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf_addr     The address of the node to split
//! @param[in]     leaf          The node to split
//! @param[inout]  parent_addr   The address of the parent of the node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling_addr  The address of the split node's new sibling
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_root(bptr_t *root,
	bptr_t leaf_addr, Node *leaf,
	bptr_t *parent_addr, Node *parent,
	bptr_t *sibling_addr, Node *sibling
) {
	// If this is the only node
	// We need to create the first inner node
	if (is_leaf(leaf_addr)) {
		// Make a new root node
		*root = MAX_LEAVES;
	} else {
		if (*root + MAX_NODES_PER_LEVEL >= MEM_SIZE) {
			return NOT_IMPLEMENTED;
		} else {
			*root = *root + MAX_NODES_PER_LEVEL;
		}
	}
	*parent_addr = *root;
	*parent = mem_read_lock(*parent_addr);
	init_node(parent);
	parent->keys[0] = leaf->keys[DIV2CEIL(TREE_ORDER)-1];
	parent->values[0].ptr = leaf_addr;
	parent->keys[1] = sibling->keys[(TREE_ORDER/2)-1];
	parent->values[1].ptr = *sibling_addr;
	return SUCCESS;
}


//! @brief Assign an allocated sibling pair below the root leve of the tree
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf_addr     The address of the node to split
//! @param[in]     leaf          The node to split
//! @param[inout]  parent_addr   The address of the parent of the node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling_addr  The address of the split node's new sibling
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_nonroot(bptr_t *root,
	bptr_t leaf_addr, Node *leaf,
	bptr_t *parent_addr, Node *parent,
	bptr_t *sibling_addr, Node *sibling
) {
	if (is_full(parent)) {
		return NOT_IMPLEMENTED;
	} else {
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			// Update key of old node
			if (parent->values[i].ptr == leaf_addr) {
				parent->keys[i] = leaf->keys[DIV2CEIL(TREE_ORDER)-1];
				// Scoot over other nodes to fit in new node
				for (li_t j = TREE_ORDER-1; j > i; --j) {
					parent->keys[j] = parent->keys[j-1];
					parent->values[j] = parent->values[j-1];
				}
				// Insert new node
				parent->keys[i+1] = sibling->keys[(TREE_ORDER/2)-1];
				parent->values[i+1].ptr = *sibling_addr;
				return SUCCESS;
			}
		}
		return NOT_IMPLEMENTED;
	}
}


//! @brief Split a node in the tree and return the affected nodes
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf_addr     The address of the node to split
//! @param[in]     leaf          The node to split
//! @param[inout]  parent_addr   The address of the parent of the node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling_addr  The address of the split node's new sibling
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_node(bptr_t *root,
	bptr_t leaf_addr, Node *leaf,
	bptr_t *parent_addr, Node *parent,
	bptr_t *sibling_addr, Node *sibling
) {
	alloc_sibling(root, leaf_addr, leaf, sibling_addr, sibling);
	if (*parent_addr == INVALID) {
		return split_root(root, leaf_addr, leaf, parent_addr, parent, sibling_addr, sibling);
	} else {
		return split_nonroot(root, leaf_addr, leaf, parent_addr, parent, sibling_addr, sibling);
	}
}


//! @brief Insert into a non-full leaf node
//! @param[in] node   The node to insert into
//! @param[in] key    The key to insert
//! @param[in] value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode insert_nonfull(Node *node, bkey_t key, bval_t value) {
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


ErrorCode insert(bptr_t *root, bkey_t key, bval_t value) {
	ErrorCode status;
	li_t i_leaf;
	Node leaf, parent, sibling;
	bptr_t leaf_addr, parent_addr, sibling_addr;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(*root, key, lineage);
	if (status != SUCCESS) return status;
	i_leaf = get_leaf_idx(lineage);
	leaf_addr = lineage[i_leaf];
	leaf = mem_read_lock(leaf_addr);
	if (i_leaf > 0) {
		parent_addr = lineage[i_leaf-1];
		parent = mem_read_lock(parent_addr);
	} else {
		parent_addr = INVALID;
	}

	// Search within the leaf node of the lineage for the key
	if (is_full(&leaf)) {
		// Try to split this node, exit on failure
		status = split_node(root, leaf_addr, &leaf, &parent_addr, &parent, &sibling_addr, &sibling);
		if (status != SUCCESS) {
			mem_unlock(leaf_addr);
			mem_unlock(sibling_addr);
			mem_unlock(parent_addr);
			return status;
		}
		// Insert the new data
		if (key < max(&leaf)) {
			status = insert_nonfull(&leaf, key, value);
		} else {
			status = insert_nonfull(&sibling, key, value);
		}
		mem_write_unlock(sibling_addr, sibling);
		mem_write_unlock(parent_addr, parent);
		if (status != SUCCESS) return status;
	} else {
		status = insert_nonfull(&leaf, key, value);
		if (parent_addr != INVALID) mem_unlock(parent_addr);
		if (status != SUCCESS) return status;
	}

	mem_write_unlock(leaf_addr, leaf);
	return SUCCESS;
}
