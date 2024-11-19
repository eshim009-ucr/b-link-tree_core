#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "types.h"


#ifndef TREE_H
#define TREE_H

typedef struct {
	bkey_t keys[TREE_ORDER];
	bval_t values[TREE_ORDER];
	bptr_t next;
} Node;

typedef struct {
	Node *memory;
	bptr_t root;
} Tree;


bool is_leaf(Tree *tree, bptr_t node_ptr);
void init_tree(Tree *tree);

ErrorCode search(Tree *tree, bkey_t key, bval_t *value);
ErrorCode insert(Tree *tree, bkey_t key, bval_t value);

#endif
