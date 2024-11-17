#include <gtest/gtest.h>
#include <cstdio>

FILE *log_stream = fopen("log.txt", "w");

TEST(InitTest, Tree) {
	Tree tree;
	init_tree(&tree);

	EXPECT_EQ(tree.root, 0);
	for (bptr_t i = 0; i < MAX_LEVELS*MAX_NODES_PER_LEVEL; ++i) {
		for (li_t j = 0; j < TREE_ORDER; ++j) {
			EXPECT_EQ(tree.memory[i].keys[j], INVALID);
		}
	}
}

#if TREE_ORDER >= 4
TEST(SearchTest, RootIsLeaf) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	init_tree(&tree);
	Node *root = &tree.memory[tree.root];
	bval_t result;

	root->keys[0] = 1; root->values[0].data = -1;
	root->keys[1] = 2; root->values[1].data = -2;
	root->keys[2] = 4; root->values[2].data = -4;
	root->keys[3] = 5; root->values[3].data = -5;
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(search(&tree, 0, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 3, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 6, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 1, &result), SUCCESS);
	EXPECT_EQ(result.data, -1);
	EXPECT_EQ(search(&tree, 2, &result), SUCCESS);
	EXPECT_EQ(result.data, -2);
	EXPECT_EQ(search(&tree, 4, &result), SUCCESS);
	EXPECT_EQ(result.data, -4);
	EXPECT_EQ(search(&tree, 5, &result), SUCCESS);
	EXPECT_EQ(result.data, -5);
}
#endif
