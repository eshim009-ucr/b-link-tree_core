#ifndef TREE_H
#define TREE_H


#include "lock.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>


//! @brief A generic node within the tree
//!
//! Can be a leaf node or an inner node
typedef struct Node {
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
	//! These may be leaf data or pointers within the tree.
	bptr_t next;
	//! @brief Used to restrict concurrent modifications to this node
	lock_t lock;
} Node;

//! @brief A memory buffer to hold tree data and the necessary metadata to
//!        interact with it
typedef struct Tree {
	//! @brief Address of the current root pointer in the memory buffer
	bptr_t root;
} Tree;


//! @brief Initialize the tree's memory buffer and root to expected default
//!        values
//! @param[in] tree  Pointer to the tree to initialize
void init_tree(Tree *tree);


#endif
