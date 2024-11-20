#ifndef IO_H
#define IO_H

//! @brief Print the contents of a tree's memory in a human-readable format
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
//! @param[in]  tree    The tree whose contents should be displayed
void dump_node_list(FILE *stream, Tree const *tree);

#endif
