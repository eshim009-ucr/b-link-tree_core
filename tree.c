#include "tree.h"
#include <string.h>


inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, MEM_SIZE*sizeof(Node));
	for (uint_fast8_t i = 0; i < MEM_SIZE; i++) {
		init_lock(&tree->memory[i].lock);
	}
	tree->root = 0;
}
