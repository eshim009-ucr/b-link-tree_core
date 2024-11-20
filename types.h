#include "defs.h"

#ifndef PRIMITIVES_H
#define PRIMITIVES_H

//! Datatype of keys
typedef uint32_t bkey_t;
//! Datatype of pointers within the tree
typedef uint32_t bptr_t;
//! Datatype of leaf data
typedef int32_t bdata_t;
//! @brief Datatype of node values, which can be either data or pointers within
//!        the tree
typedef union{
	bptr_t ptr;   //!< Internal node value which points to another node
	bdata_t data; //!< Leaf node value which holds data
} bval_t;
// Leaf index type
#if TREE_ORDER < (1 << 8)
typedef uint_fast8_t li_t;
#elif TREE_ORDER < (1 << 16)
typedef uint_fast16_t li_t;
#elif TREE_ORDER < (1 << 32)
typedef uint_fast32_t li_t;
#endif
//! @brief Status codes returned from tree functions
typedef enum {
	SUCCESS=0,
	KEY_EXISTS=1,
	NOT_IMPLEMENTED=2,
	NOT_FOUND=3,
	INVALID_ARGUMENT=4,
	OUT_OF_MEMORY=5
} ErrorCode;

#define INVALID ((bkey_t) -1)

#endif
