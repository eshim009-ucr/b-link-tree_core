#include <stdio.h>
#include "tree.h"
#include "io.h"

//! @brief Print keys of a node in a human-readable format
//!
//! Helper function for @ref dump_node_list
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
//! @param[in]  node    The node whose keys to display
void dump_keys(FILE *stream, Node const *node) {
	fprintf(stream, "[");
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3u", node->keys[i]);
		}
		if (i < TREE_ORDER-1) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "     ] ");
}

//! @brief Print values of a node in a human-readable format
//!
//! Helper function for @ref dump_node_list
//! @param[out] stream  Output stream to write to, can be a file or standard
//!                     output
//! @param[in]  node    The node whose values to display
void dump_values(FILE *stream, Node const *node) {
	fprintf(stream, "{");
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3d", node->values[i].data);
		}
		if (i < TREE_ORDER-1) {
			fprintf(stream, ", ");
		}
	}
	if (node->next == INVALID) {
		fprintf(stream, ";    ");
	} else {
		fprintf(stream, "; %3u", node->next);
	}
	fprintf(stream, "} ");
}

void dump_node_list(FILE *stream, Tree const *tree) {
	fprintf(stream, "LEAVES\n%2u ", 0);
	for (uint_fast16_t i = 0; i < MAX_LEAVES; ++i) {
		dump_keys(stream, &tree->memory[i]);
	}
	fprintf(stream, "\n   ");
	for (uint_fast16_t i = 0; i < MAX_LEAVES; ++i) {
		dump_values(stream, &tree->memory[i]);
	}
	fprintf(stream, "\n");
	fprintf(stream, "INTERNAL NODES\n");
	for (uint_fast16_t r = 1; r < (MAX_LEVELS-1); ++r) {
		fprintf(stream, "%2lu ", r*MAX_NODES_PER_LEVEL);
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_keys(stream, &tree->memory[r*MAX_NODES_PER_LEVEL + c]);
		}
		fprintf(stream, "\n   ");
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_values(stream, &tree->memory[r*MAX_NODES_PER_LEVEL + c]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n");
}
