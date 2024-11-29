#include <gtest/gtest.h>
#include <cstdio>

extern "C" {
#include "tree.h"
#include "validate.h"
#include "io.h"
};

FILE *log_stream = fopen("log.txt", "w");

TEST(InitTest, Tree) {
	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	init_tree(&tree);

	EXPECT_EQ(tree.root, 0);
	for (bptr_t i = 0; i < MAX_LEVELS*MAX_NODES_PER_LEVEL; ++i) {
		for (li_t j = 0; j < TREE_ORDER; ++j) {
			EXPECT_EQ(tree.memory[i].keys[j], INVALID);
		}
	}
}

TEST(ValidateTest, RootOneChild) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	init_tree(&tree);
	tree.root = MAX_LEAVES;
	Node *root = &tree.memory[tree.root];
	Node *lchild = &tree.memory[0];

	root->keys[0] = 6; root->values[0].ptr = 0;
	lchild->keys[0] = 1; lchild->values[0].data = -1;
	lchild->keys[1] = 2; lchild->values[1].data = -2;
	lchild->keys[2] = 4; lchild->values[2].data = -4;
	lchild->keys[3] = 5; lchild->values[3].data = -5;
	dump_node_list(log_stream, &tree);

	EXPECT_FALSE(validate(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}

#if TREE_ORDER >= 4
TEST(SearchTest, RootIsLeaf) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
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

	EXPECT_TRUE(validate(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}
#endif

#if TREE_ORDER == 4
TEST(SearchTest, OneInternal) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	init_tree(&tree);
	tree.root = MAX_LEAVES;
	Node *root = &tree.memory[tree.root];
	Node *lchild = &tree.memory[0];
	Node *rchild = &tree.memory[1];
	bval_t result;

	root->keys[0] = 6; root->values[0].ptr = 0;
	root->keys[1] = 12; root->values[1].ptr = 1;
	lchild->keys[0] = 1; lchild->values[0].data = -1;
	lchild->keys[1] = 2; lchild->values[1].data = -2;
	lchild->keys[2] = 4; lchild->values[2].data = -4;
	lchild->keys[3] = 5; lchild->values[3].data = -5;
	lchild->next = 1;
	rchild->keys[0] = 7; rchild->values[0].data = -7;
	rchild->keys[1] = 8; rchild->values[1].data = -8;
	rchild->keys[2] = 10; rchild->values[2].data = -10;
	rchild->keys[3] = 11; rchild->values[3].data = -11;
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(search(&tree, 0, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 3, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 6, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 9, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 12, &result), NOT_FOUND);
	EXPECT_EQ(search(&tree, 1, &result), SUCCESS);
	EXPECT_EQ(result.data, -1);
	EXPECT_EQ(search(&tree, 2, &result), SUCCESS);
	EXPECT_EQ(result.data, -2);
	EXPECT_EQ(search(&tree, 4, &result), SUCCESS);
	EXPECT_EQ(result.data, -4);
	EXPECT_EQ(search(&tree, 5, &result), SUCCESS);
	EXPECT_EQ(result.data, -5);
	EXPECT_EQ(search(&tree, 7, &result), SUCCESS);
	EXPECT_EQ(result.data, -7);
	EXPECT_EQ(search(&tree, 8, &result), SUCCESS);
	EXPECT_EQ(result.data, -8);
	EXPECT_EQ(search(&tree, 10, &result), SUCCESS);
	EXPECT_EQ(result.data, -10);
	EXPECT_EQ(search(&tree, 11, &result), SUCCESS);
	EXPECT_EQ(result.data, -11);

	EXPECT_TRUE(validate(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}
#endif

TEST(InsertTest, LeafNode) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	init_tree(&tree);
	Node *leaf = &tree.memory[tree.root];
	bval_t value;

	value.data = 2;
	EXPECT_EQ(insert(&tree, 0, value), SUCCESS);
	EXPECT_EQ(leaf->keys[0], 0);
	EXPECT_EQ(leaf->values[0].data, 2);
	dump_node_list(log_stream, &tree);

	value.data = 3;
	EXPECT_EQ(insert(&tree, 5, value), SUCCESS);
	EXPECT_EQ(leaf->keys[1], 5);
	EXPECT_EQ(leaf->values[1].data, 3);
	dump_node_list(log_stream, &tree);

	value.data = 1;
	EXPECT_EQ(insert(&tree, 3, value), SUCCESS);
	EXPECT_EQ(leaf->keys[1], 3);
	EXPECT_EQ(leaf->values[1].data, 1);
	EXPECT_EQ(leaf->keys[2], 5);
	EXPECT_EQ(leaf->values[2].data, 3);
	dump_node_list(log_stream, &tree);
}

TEST(InsertTest, SplitRoot) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	bval_t value;
	init_tree(&tree);

	value.data = 0;
	EXPECT_EQ(insert(&tree, 0, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	value.data = -5;
	EXPECT_EQ(insert(&tree, 5, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	value.data = -3;
	EXPECT_EQ(insert(&tree, 3, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	value.data = -1;
	EXPECT_EQ(insert(&tree, 1, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	value.data = -4;
	EXPECT_EQ(insert(&tree, 4, value), SUCCESS);
	dump_node_list(log_stream, &tree);
}

TEST(InsertTest, InsertUntilItBreaks) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree;
	Node memory[MEM_SIZE];
	tree.memory = memory;
	bval_t value;
	init_tree(&tree);

	for (uint_fast8_t i = 1; i <= 9; ++i) {
		value.data = -i;
		EXPECT_EQ(insert(&tree, i, value), SUCCESS);
		dump_node_list(log_stream, &tree);
	}
}
