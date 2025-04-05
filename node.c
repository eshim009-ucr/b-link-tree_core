#include "node.h"


bstatusval_t find_next(Node const* n, bkey_t key) {
	bstatusval_t result = {SUCCESS, {INVALID}};
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		// We overshot the node we were looking for
		// and got an uninitialized key
		if (n->keys[i] == INVALID) {
			// Empty node, error
			if (i == 0) {
				result.status = NOT_FOUND;
				return result;
			}
			// Save the last node we looked at
			else {
				result.value = n->values[i-1];
				return result;
			}
		}
		// If this key is the first key greater than what we're looking for
		// then continue down this subtree
		else if (key <= n->keys[i]) {
			result.value = n->values[i];
			return result;
		}
	}
	// Wasn't in this node, check sibling
	if (n->next == INVALID) {
		// No sibling
		result.status = NOT_FOUND;
	} else {
		result.value.ptr = n->next;
	}
	return result;
}


bstatusval_t find_value(Node const *n, bkey_t key) {
	bstatusval_t result = {SUCCESS, {INVALID}};
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (n->keys[i] == key) {
			result.value = n->values[i];
			return result;
		}
	}
	result.status = NOT_FOUND;
	return result;
}


bool is_valid(Node const *n) {
	return n->keys[0] != INVALID;
}

bool is_full(Node const *n) {
	return n->keys[TREE_ORDER-1] != INVALID;
}

void clear(Node *n) {
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		n->keys[i] = INVALID;
		n->values[i].data = INVALID;
	}
}
