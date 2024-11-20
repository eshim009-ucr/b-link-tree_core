#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "types.h"


#ifndef TREE_H
#define TREE_H

//! @brief A generic node within the tree
//!
//! Can be a leaf node or an inner node
typedef struct {
	//! @brief Keys corresponding to child data at the same indices
	//! @par Inner Nodes
	//! In inner nodes, the keys are the exclusive upper
	//! bounds to ranges of child keys such that
	//! `keys[i-1] <= child[i].keys < keys[i]`
	//! For the 0th key, \f$-\infty\f$ is the bound.
	//! @par Leaf Nodes
	//! In leaf nodes the keys are exact lookup values.
	bkey_t keys[TREE_ORDER];
	//! @brief "Pointer to" (address of) the next largest sibling node
	//!
	//! The @ref bval_t union is used to select how they are interpreted
	//! (as internal nodes that point to other nodes or leaves that hold values)
	bval_t values[TREE_ORDER];
	//! @brief The values corresponding to the keys at the same indices
	//!
	//! These may be leaf data or pointers within  the tree.
	bptr_t next;
} Node;

//! @brief A memory buffer to hold tree data and the necessary metadata to
//!        interact with it
typedef struct {
	//! @brief Pointer to memory buffer that holds the tree data
	Node *memory;
	//! @brief Address of the current root pointer in the memory buffer
	bptr_t root;
} Tree;


//! @brief Check if a node at the given address is a leaf node or an inner node
//! @param[in] tree      Pointer to the tree to check
//! @param[in] node_ptr  Address of the node within the tree to check
bool is_leaf(Tree const *tree, bptr_t node_ptr);
//! @brief Initialize the tree's memory buffer and root to expected default
//!        values
//! @param[in,out] tree Pointer to the tree to initialize
void init_tree(Tree *tree);

//! @brief Search a tree for a key
//! @param[in]  tree   The tree to search
//! @param[in]  key    The key to search for
//! @param[out] value  Pointer to hold value that was found at the given key
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode search(Tree const *tree, bkey_t key, bval_t *value);
//! @brief Insert a new value into the tree with the given key and value
//! @param[in] tree   The tree to insert the value into
//! @param[in] key    The key under which the value should be inserted
//! @param[in] value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert(Tree *tree, bkey_t key, bval_t value);

#endif
