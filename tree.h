#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef TREE_H
#define TREE_H

#define TREE_ORDER (4)

#define MAX_NODES_PER_LEVEL (10)
#define MAX_LEVELS (4)
#define MAX_LEAVES MAX_NODES_PER_LEVEL
#define MEM_SIZE (MAX_NODES_PER_LEVEL*MAX_LEVELS)

// Datatype of Keys
typedef uint32_t bkey_t;
// Datatype of pointers within the tree
typedef uint32_t bptr_t;
// Datatype of leaf values
typedef int32_t bdata_t;
// Datatype of Values
typedef union{
	bptr_t ptr;
	bdata_t data;
} bval_t;
// Leaf index type
#if TREE_ORDER < (1 << 8)
typedef uint_fast8_t li_t;
#elif TREE_ORDER < (1 << 16)
typedef uint_fast16_t li_t;
#elif TREE_ORDER < (1 << 32)
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
	bkey_t keys[TREE_ORDER];
	bval_t values[TREE_ORDER];
	bptr_t next;
} Node;

typedef struct {
	union {
		Node memory[MEM_SIZE];
		struct {
			Node leaves[MAX_LEAVES];
			Node inners[MEM_SIZE - MAX_LEAVES];
		};
	};
	bptr_t root;
} Tree;


// Descriptive shorthands for one-liners
static inline bool is_leaf(Tree *tree, bptr_t node_ptr) {
	// Assume leaves are stored at lowest memory addresses
	return node_ptr < MAX_LEAVES;
}
static inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, sizeof(tree->memory));
	tree->root = 0;
}

void print_tree(FILE *stream, Tree *tree);
void dump_node_list(FILE *stream, Tree *tree);

ErrorCode search(Tree *tree, bkey_t key, bval_t *value);
ErrorCode insert(Tree *tree, bkey_t key, bval_t value);

#endif
