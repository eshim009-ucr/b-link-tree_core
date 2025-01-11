#ifndef VALIDATE_H
#define VALIDATE_H

#include "tree.h"
#include <stdio.h>

//! @brief Check the correctness of the underlying tree assumptions
//! @param[in]  tree    The tree to check
//! @param[out] stream  An output stream to write the validation output to
//! @return `true` for passing, `false` for failing
bool validate(Tree const *tree, FILE *stream);

//! @brief Check whether all nodes of a tree are unlocked,
//!        should return true when tree is at idle
//! @param[in]  tree    The tree to check
//! @param[out] stream  An output stream to write the validation output to
//! @return `true` if all nodes are unlocked, `false` otherwise
bool is_unlocked(Tree const *tree, FILE *stream);

#endif
