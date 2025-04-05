#include "search.h"
#include "tree-helpers.h"
#include "memory.h"
#include "node.h"
#include <string.h>


bstatusval_t search(bptr_t root, bkey_t key) {
	bstatusval_t result;
	AddrNode n;
	n.addr = root;

	// Iterate until we hit a leaf
	while (!is_leaf(n.addr)) {
		n.node = mem_read(n.addr);
		result = find_next(&n.node, key);
		if (result.status != SUCCESS) return result;
		n.addr = result.value.ptr;
	}

	// Search within the leaf node of the lineage for the key
	n.node = mem_read(n.addr);
	return find_value(&n.node, key);
}
