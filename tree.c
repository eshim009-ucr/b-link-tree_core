#include <stdint.h>
#include <stdbool.h>
#include "tree.h"


//! @brief Divide by 2 and take the ceiling using only integer arithmetic
//! @param[in] x  The value to operate on
//! @return ceil(x/2)
#define DIV2CEIL(x) (((x) & 1) ? (((x)/2) + 1) : ((x)/2))

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

inline bool is_leaf(Tree const *tree, bptr_t node_ptr) {
	// Assume leaves are stored at lowest memory addresses
	return node_ptr < MAX_LEAVES;
}

//! @brief Check which level of the tree a node address resides on
//! Assumes all levels take up equal space in memory
//! @param[in] node_ptr  The node address to check
inline static bptr_t get_level(bptr_t node_ptr) {
	return (node_ptr / MAX_NODES_PER_LEVEL);
}

inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, MEM_SIZE*sizeof(Node));
	for (uint_fast8_t i = 0; i < MEM_SIZE; i++) {
		tree->memory[i].lock = (atomic_flag) ATOMIC_FLAG_INIT;
	}
	tree->root = 0;
}

//! @brief Clear a node's keys
//! @param[in] node  The node whose keys should be cleared
inline static void init_node(Node *node) {
	memset(node->keys, INVALID, TREE_ORDER*sizeof(bkey_t));
}

//! @brief Get the index of a leaf in a lineage array
//! @param[in] lineage  An existing array of a node's parents up until the root
//! @return Index of the leaf within the lineage array
inline static uint_fast8_t get_leaf_idx(bptr_t const *lineage) {
	for (uint_fast8_t i = MAX_LEVELS-1; i > 0; i--) {
		if (lineage[i] != INVALID) return i;
	}
	return 0;
}


//! @brief Helper function for traversal of a tree, used for search and insert
//! @param[in]  tree     The tree to search
//! @param[in]  key      The key to search for
//! @param[out] lineage  Pointer array to hold the parents of all nodes on the
//!                      path to the given leaf. Should be preallocated and
//!                      large enough to accommodate a tree of maximum height.
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


bstatusval_t search(Tree const *tree, bkey_t key) {
	bstatusval_t ret;
	li_t i_leaf;
	Node leaf;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	ret.status = trace_lineage(tree, key, lineage);
	// If that failed, return the relevant error code
	if (ret.status != SUCCESS) return ret;

	i_leaf = get_leaf_idx(lineage);

	// Search within the leaf node of the lineage for the key
	leaf = tree->memory[lineage[i_leaf]];
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (leaf.keys[i] == key) {
			ret.value = leaf.values[i];
			return ret;
		}
	}
	ret.status = NOT_FOUND;
	return ret;
}


//! @brief Split a node in the tree and return the affected nodes
//! @param[in]  tree          The tree the nodes reside in
//! @param[in]  old_leaf_idx  The node to split
//! @param[in]  lineage       The list of all parents and parents of parents of
//!                           this node up to and including the root node
//! @return An error code representing the success or type of failure of the
//!         operation
static ErrorCode split_node(Tree *tree, bptr_t old_leaf_idx, bptr_t const *lineage) {
	const uint_fast8_t level = get_level(old_leaf_idx);
	bptr_t new_leaf_idx;
	Node *old_leaf_node;
	Node *new_leaf_node;

	if (lineage == NULL) {
		return INVALID_ARGUMENT;
	}

	// Find an empty spot for the new leaf
	for (new_leaf_idx = level * MAX_NODES_PER_LEVEL;
		new_leaf_idx < (level+1) * MAX_NODES_PER_LEVEL;
		++new_leaf_idx) {
		// Found an empty slot
		if (tree->memory[new_leaf_idx].keys[0] == INVALID) {
			break;
		}
	}
	// If we didn't break, we didn't find an empty slot
	if (new_leaf_idx == (level+1) * MAX_NODES_PER_LEVEL) {
		return OUT_OF_MEMORY;
	}
	// Adjust next node pointers
	old_leaf_node = &tree->memory[old_leaf_idx];
	new_leaf_node = &tree->memory[new_leaf_idx];
	new_leaf_node->next = old_leaf_node->next;
	old_leaf_node->next = new_leaf_idx;
	// Move half of old node's contents to new node
	for (li_t i = 0; i < TREE_ORDER/2; ++i) {
		new_leaf_node->keys[i] = old_leaf_node->keys[i + (TREE_ORDER/2)];
		new_leaf_node->values[i] = old_leaf_node->values[i + (TREE_ORDER/2)];
		old_leaf_node->keys[i + (TREE_ORDER/2)] = INVALID;
	}
	// If this is the root node
	if (tree->root == old_leaf_idx) {
		Node *root;
		// If this is the only node
		// We need to create the first inner node
		if (is_leaf(tree, old_leaf_idx)) {
			// Make a new root node
			tree->root = MAX_LEAVES;
		} else {
			tree->root = tree->root + MAX_NODES_PER_LEVEL;
			if (tree->root >= MEM_SIZE) return NOT_IMPLEMENTED;
		}
		root = &tree->memory[tree->root];
		init_node(root);
		root->keys[0] = old_leaf_node->keys[DIV2CEIL(TREE_ORDER)-1];
		root->values[0].ptr = old_leaf_idx;
		root->keys[1] = new_leaf_node->keys[(TREE_ORDER/2)-1];
		root->values[1].ptr = new_leaf_idx;
		return SUCCESS;
	} else {
		bptr_t parent = lineage[get_leaf_idx(lineage)-1];
		if (is_full(&tree->memory[parent])) {
			return NOT_IMPLEMENTED;
		} else {
			for (li_t i = 0; i < TREE_ORDER; ++i) {
				// Update key of old node
				if (tree->memory[parent].values[i].ptr == old_leaf_idx) {
					tree->memory[parent].keys[i] = old_leaf_node->keys[DIV2CEIL(TREE_ORDER)-1];
					// Scoot over other nodse to fit in new node
					for (li_t j = TREE_ORDER-1; j > i; --j) {
						tree->memory[parent].keys[j] = tree->memory[parent].keys[j-1];
						tree->memory[parent].values[j] = tree->memory[parent].values[j-1];
					}
					// Insert new node
					tree->memory[parent].keys[i+1] = new_leaf_node->keys[(TREE_ORDER/2)-1];
					tree->memory[parent].values[i+1] .ptr = new_leaf_idx;
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
	Node *leaf;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	status = trace_lineage(tree, key, lineage);
	// If node wasn't found
	switch (status) {
		case NOT_FOUND: // Must split internal node
			bptr_t old_idx = lineage[get_leaf_idx(lineage)];
			status = split_node(tree, old_idx, lineage);
			//! TODO: Just change the last element
			trace_lineage(tree, key, lineage);
			break;
		case SUCCESS: break;
		default: return status;
	}

	for (i_leaf = MAX_LEVELS-1; i_leaf > 0; i_leaf--) {
		if (lineage[i_leaf] != INVALID) break;
	}

	// Search within the leaf node of the lineage for the key
	leaf = &tree->memory[lineage[i_leaf]];

	if (is_full(leaf)) {
		status = split_node(tree, lineage[i_leaf], lineage);
		if (status != SUCCESS) return status;
		if (key < max(leaf)) {
			status = insert_nonfull(leaf, key, value);
		} else {
			status = insert_nonfull(&tree->memory[leaf->next], key, value);
		}
		if (status != SUCCESS) return status;
	} else {
		status = insert_nonfull(leaf, key, value);
		if (status != SUCCESS) return status;
	}

	return SUCCESS;
}
