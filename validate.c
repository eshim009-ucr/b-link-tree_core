#include "validate.h"

static li_t num_children(Tree const *tree, bptr_t node) {
	Node *n = &tree->memory[node];
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (n->keys[i] == INVALID) return i;
	}
	return TREE_ORDER;
}

//! @return `true` for passing, `false` for failing
static bool validate_root(Tree const *tree, FILE *stream) {
	li_t n_child = num_children(tree, tree->root);
	bool result = is_leaf(tree, tree->root) || n_child >= 2;
	fprintf(stream, "Validating mem[%u] (root)...", tree->root);
	if (result) {
		fprintf(stream, "valid!\n");
	} else {
		fprintf(stream, "invalid,\n\t");
		fprintf(stream,
			"root is not a leaf and has too few children (%u < %u)\n",
			n_child, 2
		);
	}
	return result;
}

//! @return `true` for passing, `false` for failing
static bool validate_node(Tree const *tree, bptr_t node, FILE *stream) {
	if (node == tree->root) {
		return validate_root(tree, stream);
	} else {
		fprintf(stream, "mem[%u]...", node);
		li_t n_child = num_children(tree, node);
		bool result = is_leaf(tree, tree->root) || n_child >= TREE_ORDER / 2;
		if (result) {
			fprintf(stream, "valid!\n");
		} else {
			fprintf(stream, "invalid,\n\t");
			fprintf(stream,
				"node is not a leaf and has too few children (%u < %u)\n",
				n_child, TREE_ORDER / 2
			);
		}
		return result;
	}
}

static bool validate_children(Tree const *tree, bptr_t node, FILE *stream) {
	bool result = true;
	if (!validate_node(tree, node, stream)) {
		result = false;
	}
	if (!is_leaf(tree, node)) {
		Node *n = &tree->memory[node];
		fprintf(stream, "Validating mem[%u]'s children...\n", node);
		for (li_t i = 0; i < MAX_LEAVES; ++i) {
			fprintf(stream, "Validating child %u, ", i);
			if (n->keys[i] == INVALID) {
				fprintf(stream, "out of children\n");
				break;
			} else if (!validate_children(tree, n->values[i].ptr, stream)) {
				result = false;
			}
		}
	}
	return result;
}

bool validate(Tree const *tree, FILE *stream) {
	return validate_children(tree, tree->root, stream);
}
