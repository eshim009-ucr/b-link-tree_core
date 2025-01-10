#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tree.h"


inline void init_tree(Tree *tree) {
	memset(tree->memory, INVALID, MEM_SIZE*sizeof(Node));
	for (uint_fast8_t i = 0; i < MEM_SIZE; i++) {
		tree->memory[i].lock = (atomic_flag) ATOMIC_FLAG_INIT;
	}
	tree->root = 0;
}
