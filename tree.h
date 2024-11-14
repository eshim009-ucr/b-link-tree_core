#include <stdint.h>
#include <stdbool.h>

#ifndef TREE_H
#define TREE_H

#define TREE_ORDER (4)

// Datatype of Keys
typedef uint32_t bkey_t;
// Datatype of Values
typedef uint32_t bval_t;
// Datatype of pointers within the tree
typedef uint32_t bptr_t;
typedef enum {SUCCESS=0, KEY_EXISTS=1, NOT_IMPLEMENTED=2} ErrorCode;

#define INVALID (-1)

typedef struct {
	bkey_t keys[TREE_ORDER-1];
	bptr_t children[TREE_ORDER];
} InnerNode;
typedef struct {
	bkey_t keys[TREE_ORDER-1];
	bval_t data[TREE_ORDER-1];
	bptr_t next_leaf;
} LeafNode;

typedef union {
	InnerNode inner;
	LeafNode leaf;
} Node;

typedef struct {
	uint_fast32_t memorySize;
	Node *memory;
} Tree;

inline void init_leaf(LeafNode *leaf);
ErrorCode insert(LeafNode *leaf, bkey_t key, bval_t value);

#endif
