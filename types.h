#ifndef PRIMITIVES_H
#define PRIMITIVES_H


#include "defs.h"
#include <stdint.h>


//! Datatype of keys
typedef uint32_t bkey_t;
//! Datatype of pointers within the tree
typedef uint32_t bptr_t;
//! Datatype of leaf data
typedef int32_t bdata_t;
//! @brief Datatype of node values, which can be either data or pointers within
//!        the tree
typedef union {
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
//! Explanation: https://en.wikipedia.org/wiki/X_macro
#define ERROR_CODE_XMACRO \
	X(SUCCESS, 0) \
	X(KEY_EXISTS, 1) \
	X(NOT_IMPLEMENTED, 2) \
	X(NOT_FOUND, 3) \
	X(INVALID_ARGUMENT, 4) \
	X(OUT_OF_MEMORY, 5)
//! @brief Status codes returned from tree functions
typedef enum {
#define X(codename, codeval) codename = codeval,
ERROR_CODE_XMACRO
#undef X
} ErrorCode;
//! @brief Names of status codes, used for error messages
static const char *const ERROR_CODE_NAMES[] = {
#define X(codename, codeval) #codename,
ERROR_CODE_XMACRO
#undef X
};
//! @brief Result of an operation returning a value and a return code
typedef struct {
	uint_least8_t status;
	bval_t value;
} bstatusval_t;

#define INVALID ((bkey_t) -1)

#endif
