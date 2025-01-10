#ifndef SEARCH_H
#define SEARCH_H

#include "tree.h"

//! @brief Search a tree for a key
//! @param[in]  tree   The tree to search
//! @param[in]  key    The key to search for
//! @return Struct containing requested data on success and an error code
bstatusval_t search(Tree const *tree, bkey_t key);

#endif
