#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef TREE_H
#define TREE_H

#define MAX_KEYS (4)
#define MAX_CHILDREN (MAX_KEYS+1)

#define MAX_NODES_PER_LEVEL (10)
#define MAX_LEVELS (4)
#define MAX_LEAVES MAX_NODES_PER_LEVEL

// Datatype of Keys
typedef uint32_t bkey_t;
// Datatype of Values
typedef uint32_t bval_t;
// Datatype of pointers within the tree
typedef uint32_t bptr_t;
// Leaf index type
#if MAX_CHILDREN < (1 << 8)
typedef uint_fast8_t li_t;
#elif MAX_CHILDREN < (1 << 16)
typedef uint_fast16_t li_t;
#elif MAX_CHILDREN < (1 << 32)
typedef uint_fast32_t li_t;
#endif
// Function error codes
typedef enum {
	SUCCESS=0,
	KEY_EXISTS=1,
	NOT_IMPLEMENTED=2,
	NOT_FOUND=3,
	INVALID_ARGUMENT=4,
	OUT_OF_MEMORY=5
} ErrorCode;

#define INVALID ((bkey_t) -1)


typedef struct {
	bkey_t keys[MAX_KEYS];
	bptr_t children[MAX_CHILDREN];
} InnerNode;

typedef struct {
	bkey_t keys[MAX_KEYS];
	bval_t data[MAX_KEYS];
	bptr_t next_leaf;
} LeafNode;

typedef union {
	InnerNode inner;
	LeafNode leaf;
} Node;


typedef struct {
	union {
		Node memory[MAX_NODES_PER_LEVEL*MAX_LEVELS];
		struct {
			LeafNode leaves[MAX_NODES_PER_LEVEL];
			InnerNode inners[MAX_NODES_PER_LEVEL*(MAX_LEVELS-1)];
		};
	};
	bptr_t root;
} Tree;


// Descriptive shorthands for one-liners
static inline bptr_t to_bptr(Tree *tree, Node *node) {
	return (node - tree->memory) / sizeof(Node);
}
static inline bool is_leaf(Tree *tree, Node *node) {
	// Assume leaves are stored at lowest memory addresses
	return (void*) node < (void*) tree->inners;
}
static inline Node* get_root(Tree *tree) {
	return &tree->memory[tree->root];
}
static inline bool root_is_leaf(Tree *tree) {
	return tree->root < MAX_LEAVES;
}
static inline void init_node(Node *node) {
	memset(node->inner.keys, INVALID, sizeof(node->inner.keys));
}
static inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, sizeof(tree->memory));
	tree->root = 0;
}

void print_tree(FILE *stream, Tree *tree);
void dump_node_list(FILE *stream, Tree *tree);

ErrorCode insert(Tree *tree, bkey_t key, bval_t value);

#endif
