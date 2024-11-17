#include <stdint.h>
#include <stdbool.h>
#include "tree.h"


ErrorCode search(Tree *tree, bkey_t key, bval_t *value) {
	bptr_t current = tree->root;
	Node node;

	while (!is_leaf(tree, current)) {
		node = tree->memory[current];
		// Search internal node
		for (li_t i = 0; i < TREE_ORDER; ++i) {
			// We overshot the maximum bound, so the last one is what we want
			// Invalid key is all 1s, so will always be >= key
			if (node.keys[i] >= key) {
				current = node.values[i].ptr;
				continue;
			}
		}
		// Wasn't in this node, check sibling
		if (node.next == INVALID) {
			return NOT_FOUND;
		}
		current = node.next;
	}

	node = tree->memory[current];
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (node.keys[i] == key) {
			if (value) *value = node.values[i];
			return SUCCESS;
		}
	}
	return NOT_FOUND;
}


ErrorCode insert(Tree *tree, bkey_t key, bval_t value) {
	return NOT_IMPLEMENTED;
}
