#ifndef INSERT_HELPERS_H
#define INSERT_HELPERS_H


#include "node.h"


//! @brief Clear a node's keys
//! @param[in] node  The node whose keys should be cleared
void init_node(Node *node);

//! @brief Find the maximum key in a node
//! @param[in] node  The node to check
//! @return The largest valid key in the node
bkey_t max(Node *node);

//! @brief Allocate a new sibling node in an empty slot in main mameory
//!
//! Acquires a lock on the sibling node
ErrorCode alloc_sibling(
	//! [in] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling
);

//! @brief Assign an allocated sibling pair at the root leve of the tree
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode split_root(
	//! [in] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling
);


//! @brief Assign an allocated sibling pair below the root leve of the tree
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode split_nonroot(
	//! [in] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling
);


//! @brief Split a node in the tree and return the affected nodes
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode split_node(
	//! [in] Root of the tree the nodes reside in
	bptr_t *root,
	//! [in] The node to split
	AddrNode *leaf,
	//! [inout] The parent of the node to split
	AddrNode *parent,
	//! [out] The contents of the split node's new sibling
	AddrNode *sibling
);


//! @brief Insert into a non-full leaf node
//! @param[in] node   The node to insert into
//! @param[in] key    The key to insert
//! @param[in] value  The value to insert
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert_nonfull(Node *node, bkey_t key, bval_t value);


//! @brief Insert new data into a node or its newly created sibling
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode insert_after_split(
	//! [in] The key to insert
	bkey_t key,
	//! [in] The value to insert
	bval_t value,
	//! [out] The original node to insert into
	AddrNode *leaf,
	//! [out] The new sibling to insert into
	AddrNode *sibling
);

//! @brief Replace a key without changing its corresponding value
//!
//! Helper for adjusting the high key after splitting nodes
//! @return An error code representing the success or type of failure of the
//!         operation
ErrorCode rekey(Node *node, bkey_t old_key, bkey_t new_key);

#endif
