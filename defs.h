#ifndef DEFS_H
#define DEFS_H

//! Number of children in each node of a tree
#define TREE_ORDER (4)
//! "Width" of a memory grid
//! Each level of the tree is on its own level
#define MAX_NODES_PER_LEVEL (10)
//! Maximum height of a tree
#define MAX_LEVELS (4)
//! Maximum number of leaf nodes in the entire tree
#define MAX_LEAVES MAX_NODES_PER_LEVEL
//! Maximum number of nodes that can be stored in a tree's memory
#define MEM_SIZE (MAX_NODES_PER_LEVEL * MAX_LEVELS)

#endif
