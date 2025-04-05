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
//! @param[in]     leaf          The node to split
//! @param[out]    sibling       The contents of the split node's new sibling
//!
//! Acquires a lock on the sibling node
static ErrorCode alloc_sibling(
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


//! @brief Assign an allocated sibling pair at the root leve of the tree
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf          The node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_root(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	// If this is the only node
	// We need to create the first inner node
	if (is_leaf(leaf->addr)) {
		// Make a new root node
		*root = MAX_LEAVES;
	} else {
		if (*root + MAX_NODES_PER_LEVEL >= MEM_SIZE) {
			return NOT_IMPLEMENTED;
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


//! @brief Assign an allocated sibling pair below the root leve of the tree
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf          The node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_nonroot(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	if (is_full(&parent->node)) {
		return NOT_IMPLEMENTED;
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


//! @brief Split a node in the tree and return the affected nodes
//! @param[in]     root          Root of the tree the nodes reside in
//! @param[in]     leaf          The node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_node(
	bptr_t *root, AddrNode *leaf, AddrNode *parent, AddrNode *sibling
) {
	alloc_sibling(root, leaf, sibling);
	if (parent->addr == INVALID) {
		return split_root(root, leaf, parent, sibling);
	} else {
		return split_nonroot(root, leaf, parent, sibling);
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
	AddrNode leaf, parent, sibling;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(*root, key, lineage);
	if (status != SUCCESS) return status;
	i_leaf = get_leaf_idx(lineage);
	leaf.addr = lineage[i_leaf];
	leaf.node = mem_read_lock(leaf.addr);
	if (i_leaf > 0) {
		parent.addr = lineage[i_leaf-1];
		parent.node = mem_read_lock(parent.addr);
	} else {
		parent.addr = INVALID;
	}

	// Search within the leaf node of the lineage for the key
	if (is_full(&leaf.node)) {
		// Try to split this node, exit on failure
		status = split_node(root, &leaf, &parent, &sibling);
		if (status != SUCCESS) {
			mem_unlock(leaf.addr);
			mem_unlock(sibling.addr);
			mem_unlock(parent.addr);
			return status;
		}
		// Insert the new data
		if (key < max(&leaf.node)) {
			status = insert_nonfull(&leaf.node, key, value);
		} else {
			status = insert_nonfull(&sibling.node, key, value);
		}
		mem_write_unlock(sibling.addr, sibling.node);
		mem_write_unlock(parent.addr, parent.node);
		if (status != SUCCESS) return status;
	} else {
		status = insert_nonfull(&leaf.node, key, value);
		if (parent.addr != INVALID) mem_unlock(parent.addr);
		if (status != SUCCESS) return status;
	}

	mem_write_unlock(leaf.addr, leaf.node);
	return SUCCESS;
}
