#ifndef INSERT_H
#define INSERT_H

#include "tree.h"

//! @brief Insert a new value into the tree with the given key and value
//! @param[in] tree   The tree to insert the value into
//! @param[in] key    The key under which the value should be inserted
//! @param[in] value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert(Tree *tree, bkey_t key, bval_t value);

#endif
