#include "search.h"
#include "tree-helpers.h"
#include "memory.h"
#include "node.h"
#include <string.h>


bstatusval_t search(bptr_t root, bkey_t key) {
	bstatusval_t ret;
	li_t i_leaf;
	Node leaf;
	bptr_t lineage[MAX_LEVELS];

	// Initialize lineage array
	memset(lineage, INVALID, MAX_LEVELS*sizeof(bptr_t));
	// Try to trace lineage
	ret.status = trace_lineage(root, key, lineage);
	// If that failed, return the relevant error code
	if (ret.status != SUCCESS) return ret;

	i_leaf = get_leaf_idx(lineage);

	// Search within the leaf node of the lineage for the key
	leaf = mem_read(lineage[i_leaf]);
	for (li_t i = 0; i < TREE_ORDER; ++i) {
		if (leaf.keys[i] == key) {
			ret.value = leaf.values[i];
			return ret;
		}
	}
	ret.status = NOT_FOUND;
	return ret;
}
