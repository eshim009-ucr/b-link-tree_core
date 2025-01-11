#ifndef TREE_HELPERS_H
#define TREE_HELPERS_H


#include "tree.h"


//! @brief "Address to Struct", converts a `bptr_t` to a `Node*`.
//! Assumed to be with respect to a local tree pointer named "tree"
#define A2S(node) (tree->memory[node])


//! @brief Check if a node at the given address is a leaf node or an inner node
//! @param[in] tree      Pointer to the tree to check
//! @param[in] node_ptr  Address of the node within the tree to check
inline static bool is_leaf(Tree const *tree, bptr_t node_ptr) {
	// Assume leaves are stored at lowest memory addresses
	return node_ptr < MAX_LEAVES;
}

//! @brief Check which level of the tree a node address resides on
//! Assumes all levels take up equal space in memory
//! @param[in] node_ptr  The node address to check
inline static bptr_t get_level(bptr_t node_ptr) {
	return (node_ptr / MAX_NODES_PER_LEVEL);
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
ErrorCode trace_lineage(Tree const *tree, bkey_t key, bptr_t *lineage);


#endif
