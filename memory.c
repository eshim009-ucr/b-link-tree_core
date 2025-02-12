#ifndef __SYNTHESIS__
#include "lock.h"
#include "memory.h"
#include "tree.h"
#include <string.h>


static Node memory[MEM_SIZE];


Node mem_read(bptr_t address) {
	return memory[address];
}

Node mem_read_lock(bptr_t address) {
	lock_p(&memory[address].lock);
	return mem_read(address);
}

void mem_write_unlock(bptr_t address, Node node) {
	lock_v(&node.lock);
	memory[address] = node;
}

void mem_unlock(bptr_t address) {
	lock_v(&memory[address].lock);
}

void mem_reset_all() {
	memset(memory, INVALID, MEM_SIZE*sizeof(Node));
	for (uint_fast8_t i = 0; i < MEM_SIZE; i++) {
		init_lock(&memory[i].lock);
	}
}

bptr_t ptr_to_addr(void *ptr) {
	return (ptr-(void*)memory)/sizeof(Node);
}


#endif
