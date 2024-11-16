#include <stdio.h>
#include "tree.h"

void dump_keys(Node *node) {
	printf("[");
	for (li_t i = 0; i < MAX_KEYS; ++i) {
		if (node->inner.keys[i] == INVALID) {
			printf("   ");
		} else {
			printf("%3u", node->inner.keys[i]);
		}
		if (i < MAX_KEYS-1) {
			printf(", ");
		}
	}
	printf("     ] ");
}

void dump_inner(InnerNode *node) {
	printf("{");
	for (li_t i = 0; i < MAX_CHILDREN; ++i) {
		if (i == MAX_KEYS || node->keys[i] == INVALID) {
			printf("   ");
		} else {
			printf("%3u", node->children[i]);
		}
		if (i < MAX_CHILDREN-1) {
			printf(", ");
		}
	}
	printf("} ");
}

void dump_leaf(LeafNode *node) {
	printf("{");
	for (li_t i = 0; i < MAX_KEYS; ++i) {
		if (node->keys[i] == INVALID) {
			printf("   ");
		} else {
			printf("%3d", node->data[i]);
		}
		if (i < MAX_KEYS-1) {
			printf(", ");
		}
	}
	if (node->next_leaf == INVALID) {
		printf(";    ");
	} else {
		printf("; %3u", node->next_leaf);
	}
	printf("} ");
}

void dump_node_list(Tree *tree) {
	printf("LEAVES\n%2u ", 0);
	for (uint_fast16_t i = 0; i < MAX_NODES_PER_LEVEL; ++i) {
		dump_keys((Node*) &tree->leaves[i]);
	}
	printf("\n   ");
	for (uint_fast16_t i = 0; i < MAX_NODES_PER_LEVEL; ++i) {
		dump_leaf(&tree->leaves[i]);
	}
	printf("\n");
	printf("INNERS\n");
	for (uint_fast16_t r = 0; r < (MAX_LEVELS-1); ++r) {
		printf("%2u ", r+1);
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_keys((Node*) &tree->inners[r*MAX_NODES_PER_LEVEL + c]);
		}
		printf("\n   ");
		for (uint_fast16_t c = 0; c < MAX_NODES_PER_LEVEL; ++c) {
			dump_inner(&tree->inners[r*MAX_NODES_PER_LEVEL + c]);
		}
		printf("\n");
	}
	printf("\n");
}
