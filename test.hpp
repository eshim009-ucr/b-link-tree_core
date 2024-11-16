#include <gtest/gtest.h>

TEST(InitTest, Tree) {
	Tree tree;
	init_tree(&tree);

	EXPECT_EQ(tree.root, 0);
	for (bptr_t i = 0; i < MAX_LEVELS*MAX_NODES_PER_LEVEL; ++i) {
		for (li_t j = 0; j < MAX_KEYS; ++j) {
			EXPECT_EQ(tree.memory[i].inner.keys[j], INVALID);
		}
	}
}

TEST(InlinesTest, IsLeaf) {
	Tree tree;
	for (bptr_t i = 0; i < MAX_NODES_PER_LEVEL; ++i) {
		EXPECT_TRUE(is_leaf(&tree, (Node*) &tree.leaves[i]));
	}
	for (bptr_t i = 0; i < MAX_NODES_PER_LEVEL*MAX_LEVELS; ++i) {
		EXPECT_FALSE(is_leaf(&tree, (Node*) &tree.inners[i]));
	}
}

TEST(InsertTest, LeafNode) {
	Tree tree;
	LeafNode *leaf = &tree.leaves[0];
	init_tree(&tree);

	EXPECT_EQ(insert(&tree, 0, 2), SUCCESS);
	EXPECT_EQ(leaf->keys[0], 0);
	EXPECT_EQ(leaf->data[0], 2);

	EXPECT_EQ(insert(&tree, 5, 3), SUCCESS);
	EXPECT_EQ(leaf->keys[1], 5);
	EXPECT_EQ(leaf->data[1], 3);

	EXPECT_EQ(insert(&tree, 3, 1), SUCCESS);
	EXPECT_EQ(leaf->keys[1], 3);
	EXPECT_EQ(leaf->data[1], 1);
	EXPECT_EQ(leaf->keys[2], 5);
	EXPECT_EQ(leaf->data[2], 3);
}

TEST(InsertTest, SplitRoot) {
	Tree tree;
	init_tree(&tree);

	EXPECT_EQ(insert(&tree, 0, 0), SUCCESS);
	dump_node_list(&tree);
	EXPECT_EQ(insert(&tree, 5, -5), SUCCESS);
	dump_node_list(&tree);
	EXPECT_EQ(insert(&tree, 3, -3), SUCCESS);
	dump_node_list(&tree);
	EXPECT_EQ(insert(&tree, 1, -1), SUCCESS);
	dump_node_list(&tree);
	EXPECT_EQ(insert(&tree, 4, -4), SUCCESS);
	dump_node_list(&tree);
}
