#include <stdio.h>
#include "tree.h"

void dump_keys(FILE *stream, Node *node) {
	fprintf(stream, "[");
	for (li_t i = 0; i < MAX_KEYS; ++i) {
		if (node->inner.keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3u", node->inner.keys[i]);
		}
		if (i < MAX_KEYS-1) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "     ] ");
}

void dump_inner(FILE *stream, InnerNode *node) {
	fprintf(stream, "{");
	for (li_t i = 0; i < MAX_CHILDREN; ++i) {
		if (i == MAX_KEYS || node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3u", node->children[i]);
		}
		if (i < MAX_CHILDREN-1) {
			fprintf(stream, ", ");
		}
	}
	fprintf(stream, "} ");
}

void dump_leaf(FILE *stream, LeafNode *node) {
	fprintf(stream, "{");
	for (li_t i = 0; i < MAX_KEYS; ++i) {
		if (node->keys[i] == INVALID) {
			fprintf(stream, "   ");
		} else {
			fprintf(stream, "%3d", node->data[i]);
		}
		if (i < MAX_KEYS-1) {
			fprintf(stream, ", ");
		}
	}
	if (node->next_leaf == INVALID) {
		fprintf(stream, ";    ");
	} else {
		fprintf(stream, "; %3u", node->next_leaf);
	}
	fprintf(stream, "} ");
}

void dump_node_list(FILE *stream, Tree *tree) {
	fprintf(stream, "LEAVES\n%2u ", 0);
	for (uint_fast16_t i = 0; i < MAX_NODES_PER_LEVEL; ++i) {
		dump_keys(stream, (Node*) &tree->leaves[i]);
	}
	fprintf(stream, "\n   ");
	for (uint_fast16_t i = 0; i < MAX_NODES_PER_LEVEL; ++i) {
		dump_leaf(stream, &tree->leaves[i]);
	}
	fprintf(stream, "\n");
	fprintf(stream, "INNERS\n");
	for (uint_fast16_t r = 0; r < (MAX_LEVELS-1); ++r) {
		fprintf(stream, "%2u ", (r+1)*MAX_NODES_PER_LEVEL);
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_keys(stream, (Node*) &tree->inners[r*MAX_NODES_PER_LEVEL + c]);
		}
		fprintf(stream, "\n   ");
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_inner(stream, &tree->inners[r*MAX_NODES_PER_LEVEL + c]);
		}
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n");
}
