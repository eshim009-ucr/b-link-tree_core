#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tree.h"


inline void init_leaf(LeafNode *leaf) {
	memset(leaf->keys, INVALID, sizeof(leaf->keys));
}

ErrorCode insert(LeafNode *leaf, bkey_t key, bval_t value) {
	uint_fast8_t i_empty = 0;
	uint_fast8_t i_insert = 0;

	for (uint_fast8_t i = 0; i < TREE_ORDER-1; ++i) {
		if (leaf->keys[i] == INVALID) {
			// Scoot nodes if necessary to maintain ordering
			for (; i_insert < i; i--) {
				leaf->keys[i] = leaf->keys[i-1];
				leaf->data[i] = leaf->data[i-1];
			}
			// Do the actual insertion
			leaf->keys[i_insert] = key;
			leaf->data[i_insert] = value;
			return SUCCESS;
		} else if (leaf->keys[i] == key) {
			return KEY_EXISTS;
		} else if (leaf->keys[i] < key) {
			i_insert++;
		}
	}
	// We only get to this part if the leaf is full

	return NOT_IMPLEMENTED;
}
