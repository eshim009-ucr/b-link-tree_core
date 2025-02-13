#include "insert.h"
#include "tree-helpers.h"
#include "tree.h"
#include "memory.h"
#include <stddef.h>
#include <string.h>


#include <stdio.h>
static int leaf_ctr = 0;
static int sibling_ctr = 0;
static int parent_ctr = 0;


//! @brief Divide by 2 and take the ceiling using only integer arithmetic
//! @param[in] x  The value to operate on
//! @return ceil(x/2)
#define DIV2CEIL(x) (((x) & 1) ? (((x)/2) + 1) : ((x)/2))


//! @brief Clear a node's keys
//! @param[in] node  The node whose keys should be cleared
inline static void init_node(Node *node) {
	memset(node->keys, INVALID, TREE_ORDER*sizeof(bkey_t));
}

//! @brief Check if all keys in a node are in use
//! @param[in] node  The node to check
//! @return True if all keys are in use, false otherwise
inline static bool is_full(Node *node) {
	return node->keys[TREE_ORDER-1] != INVALID;
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


//! @brief Split a node in the tree and return the affected nodes
//! @param[in]     tree          The tree the nodes reside in
//! @param[in]     leaf_addr     The address of the node to split
//! @param[in]     leaf          The node to split
//! @param[inout]  parent_addr   The address of the parent of the node to split
//! @param[inout]  parent        The parent of the node to split
//! @param[out]    sibling_addr  The address of the split node's new sibling
//! @param[out]    sibling       The contents of the split node's new sibling
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_node(Tree *tree,
	bptr_t leaf_addr, Node *leaf,
	bptr_t *parent_addr, Node *parent,
	bptr_t *sibling_addr, Node *sibling
) {
	const uint_fast8_t level = get_level(leaf_addr);

	printf("Splitting mem[%d]\n", leaf_addr);
	// Find an empty spot for the new leaf
	for (*sibling_addr = level * MAX_NODES_PER_LEVEL;
		*sibling_addr < (level+1) * MAX_NODES_PER_LEVEL;
		++*sibling_addr) {
		// Found an empty slot
		if (mem_read(*sibling_addr).keys[0] == INVALID) {
			break;
		}
	}
	printf("Locking(%d) sibling (mem[%d]) on line 72 in split_node\n", ++sibling_ctr, *sibling_addr);
	*sibling = mem_read_lock(*sibling_addr);
	// If we didn't break, we didn't find an empty slot
	if (*sibling_addr == (level+1) * MAX_NODES_PER_LEVEL) {
		printf("Unlocking(%d) sibling (mem[%d]) on line 76 in split_node\n", --sibling_ctr, *sibling_addr);
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
	// If this is the root node
	if (*parent_addr == INVALID) {
		// If this is the only node
		// We need to create the first inner node
		if (is_leaf(tree, leaf_addr)) {
			// Make a new root node
			tree->root = MAX_LEAVES;
		} else {
			tree->root = tree->root + MAX_NODES_PER_LEVEL;
			if (tree->root >= MEM_SIZE) return NOT_IMPLEMENTED;
		}
		*parent_addr = tree->root;
		printf("Locking(%d) parent (mem[%d]) on line 101 in split_node\n", ++parent_ctr, *parent_addr);
		*parent = mem_read_lock(*parent_addr);
		init_node(parent);
		parent->keys[0] = leaf->keys[DIV2CEIL(TREE_ORDER)-1];
		parent->values[0].ptr = leaf_addr;
		parent->keys[1] = sibling->keys[(TREE_ORDER/2)-1];
		parent->values[1].ptr = *sibling_addr;
		return SUCCESS;
	} else {
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
					printf("Unlocking(%d) parent (mem[%d]) on line 125\n", --parent_ctr, *parent_addr);
					mem_write_unlock(*parent_addr, *parent);
					return SUCCESS;
				}
			}
			return NOT_IMPLEMENTED;
		}
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
		if (node->keys[i] == INVALID) {
			// Scoot nodes if necessary to maintain ordering
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


ErrorCode insert(Tree *tree, bkey_t key, bval_t value) {
	ErrorCode status;
	li_t i_leaf;
	Node leaf, parent, sibling;
	bptr_t leaf_addr, parent_addr, sibling_addr;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(tree, key, lineage);
	i_leaf = get_leaf_idx(lineage);
	leaf_addr = lineage[i_leaf];
	printf("Locking(%d) leaf (mem[%d]) on line 179 in insert\n", ++leaf_ctr, leaf_addr);
	leaf = mem_read_lock(leaf_addr);
	if (i_leaf > 0) {
		parent_addr = lineage[i_leaf-1];
		printf("Locking(%d) parent (mem[%d]) on line 183 in insert\n", ++parent_ctr, parent_addr);
		parent = mem_read_lock(parent_addr);
	} else {
		parent_addr = INVALID;
	}
	// If node wasn't found
	switch (status) {
		case NOT_FOUND: // Must split internal node
			status = split_node(tree, leaf_addr, &leaf, &parent_addr, &parent, &sibling_addr, &sibling);
			//! TODO: Just change the last element
			trace_lineage(tree, key, lineage);
			break;
		case SUCCESS: break;
		default:
			printf("Locking(%d) leaf (mem[%d]) on line 197 in insert\n", ++leaf_ctr, leaf_addr);
			mem_unlock(leaf_addr);
			return status;
	}

	// Search within the leaf node of the lineage for the key
	if (is_full(&leaf)) {
		// Try to split this node, exit on failure
		status = split_node(tree, leaf_addr, &leaf, &parent_addr, &parent, &sibling_addr, &sibling);
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
		printf("Unlocking(%d) sibling (mem[%d]) on line 218 in insert\n", --sibling_ctr, sibling_addr);
		mem_write_unlock(sibling_addr, sibling);
		printf("Unlocking(%d) parent (mem[%d]) on line 2220 in insert\n", --parent_ctr, parent_addr);
		mem_write_unlock(parent_addr, parent);
		if (status != SUCCESS) return status;
	} else {
		status = insert_nonfull(&leaf, key, value);
		if (parent_addr != INVALID) mem_unlock(parent_addr);
		if (status != SUCCESS) return status;
	}

	printf("Unlocking(%d) leaf (mem[%d]) on line 229 in insert\n", --leaf_ctr, leaf_addr);
	mem_write_unlock(leaf_addr, leaf);
	return SUCCESS;
}
