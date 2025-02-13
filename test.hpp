#include <gtest/gtest.h>
#include <cstdio>

extern "C" {
#include "tree.h"
#include "insert.h"
#include "search.h"
#include "validate.h"
#include "io.h"
#include "memory.h"
};

FILE *log_stream = fopen("log.txt", "w");

TEST(InitTest, Tree) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = 0};
	mem_reset_all();

	EXPECT_EQ(tree.root, 0);
	for (bptr_t i = 0; i < MAX_LEVELS*MAX_NODES_PER_LEVEL; ++i) {
		Node n = mem_read(i);
		for (li_t j = 0; j < TREE_ORDER; ++j) {
			EXPECT_EQ(n.keys[j], INVALID);
		}
	}

	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(ValidateTest, RootOneChild) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = MAX_LEAVES};
	mem_reset_all();
	Node root = mem_read_lock(tree.root);
	Node lchild = mem_read_lock(0);

	root.keys[0] = 6; root.values[0].ptr = 0;
	lchild.keys[0] = 1; lchild.values[0].data = -1;
	lchild.keys[1] = 2; lchild.values[1].data = -2;
	lchild.keys[2] = 4; lchild.values[2].data = -4;
	lchild.keys[3] = 5; lchild.values[3].data = -5;
	mem_write_unlock(tree.root, root);
	mem_write_unlock(0, lchild);
	dump_node_list(log_stream, &tree);

	EXPECT_FALSE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}

#if TREE_ORDER >= 4
TEST(SearchTest, RootIsLeaf) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = 0};
	mem_reset_all();
	Node root = mem_read_lock(tree.root);
	bstatusval_t result;

	root.keys[0] = 1; root.values[0].data = -1;
	root.keys[1] = 2; root.values[1].data = -2;
	root.keys[2] = 4; root.values[2].data = -4;
	root.keys[3] = 5; root.values[3].data = -5;
	mem_write_unlock(tree.root, root);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(search(&tree, 0).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 3).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 6).status, NOT_FOUND);
	result = search(&tree, 1);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(&tree, 2);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(&tree, 4);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(&tree, 5);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);

	EXPECT_TRUE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
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

	Tree tree = {.root = MAX_LEAVES};
	mem_reset_all();
	Node root = mem_read_lock(tree.root);
	Node lchild = mem_read_lock(0);
	Node rchild = mem_read_lock(1);
	bstatusval_t result;

	root.keys[0] = 6; root.values[0].ptr = 0;
	root.keys[1] = 12; root.values[1].ptr = 1;
	lchild.keys[0] = 1; lchild.values[0].data = -1;
	lchild.keys[1] = 2; lchild.values[1].data = -2;
	lchild.keys[2] = 4; lchild.values[2].data = -4;
	lchild.keys[3] = 5; lchild.values[3].data = -5;
	lchild.next = 1;
	rchild.keys[0] = 7; rchild.values[0].data = -7;
	rchild.keys[1] = 8; rchild.values[1].data = -8;
	rchild.keys[2] = 10; rchild.values[2].data = -10;
	rchild.keys[3] = 11; rchild.values[3].data = -11;
	mem_write_unlock(tree.root, root);
	mem_write_unlock(0, lchild);
	mem_write_unlock(1, rchild);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(search(&tree, 0).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 3).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 6).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 9).status, NOT_FOUND);
	EXPECT_EQ(search(&tree, 12).status, NOT_FOUND);
	result = search(&tree, 1);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(&tree, 2);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(&tree, 4);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(&tree, 5);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);
	result = search(&tree, 7);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -7);
	result = search(&tree, 8);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -8);
	result = search(&tree, 10);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -10);
	result = search(&tree, 11);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -11);

	EXPECT_TRUE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}
#endif

TEST(InsertTest, LeafNode) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = 0};
	mem_reset_all();
	bval_t value;

	value.data = 2;
	EXPECT_EQ(insert(&tree, 0, value), SUCCESS);
	EXPECT_EQ(mem_read(tree.root).keys[0], 0);
	EXPECT_EQ(mem_read(tree.root).values[0].data, 2);
	dump_node_list(log_stream, &tree);

	value.data = 3;
	EXPECT_EQ(insert(&tree, 5, value), SUCCESS);
	EXPECT_EQ(mem_read(tree.root).keys[1], 5);
	EXPECT_EQ(mem_read(tree.root).values[1].data, 3);
	dump_node_list(log_stream, &tree);

	value.data = 1;
	EXPECT_EQ(insert(&tree, 3, value), SUCCESS);
	EXPECT_EQ(mem_read(tree.root).keys[1], 3);
	EXPECT_EQ(mem_read(tree.root).values[1].data, 1);
	EXPECT_EQ(mem_read(tree.root).keys[2], 5);
	EXPECT_EQ(mem_read(tree.root).values[2].data, 3);
	dump_node_list(log_stream, &tree);

	EXPECT_TRUE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SplitRoot) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = 0};
	bval_t value;
	mem_reset_all();

	value.data = 0;
	EXPECT_EQ(insert(&tree, -value.data, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(mem_read(tree.root).keys[0], 0);
	EXPECT_EQ(mem_read(tree.root).values[0].data, value.data);
	value.data = -5;
	EXPECT_EQ(insert(&tree, -value.data, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(mem_read(tree.root).keys[1], -value.data);
	EXPECT_EQ(mem_read(tree.root).values[1].data, value.data);
	value.data = -3;
	EXPECT_EQ(insert(&tree, -value.data, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(mem_read(tree.root).keys[1], -value.data);
	EXPECT_EQ(mem_read(tree.root).values[1].data, value.data);
	value.data = -1;
	EXPECT_EQ(insert(&tree, -value.data, value), SUCCESS);
	dump_node_list(log_stream, &tree);
	EXPECT_EQ(mem_read(1).keys[1], -value.data);
	EXPECT_EQ(mem_read(1).values[1].data, value.data);
	value.data = -4;
	EXPECT_EQ(insert(&tree, -value.data, value), SUCCESS);
	dump_node_list(log_stream, &tree);

	EXPECT_TRUE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, InsertUntilItBreaks) {
	const testing::TestInfo* const test_info
		= testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	Tree tree = {.root = 0};
	bval_t value;
	mem_reset_all();

	// Insert values
	for (uint_fast8_t i = 1; i <= 17; ++i) {
		value.data = -i;
		printf("Inserting %d\n", i);
		ASSERT_EQ(insert(&tree, i, value), SUCCESS);
		dump_node_list(log_stream, &tree);
	}
	// Check that they're instantiated in memory correctly
	uint_fast8_t next = 1;
	for (bptr_t i = 0; i < MAX_LEAVES; i++) {
		for (li_t j = 0; j < TREE_ORDER; ++j) {
			if (mem_read(i).keys[j] == INVALID) {
				break;
			} else {
				EXPECT_EQ(mem_read(i).keys[j], next);
				EXPECT_EQ(mem_read(i).values[j].data, -next);
				next++;
			}
		}
	}

	EXPECT_TRUE(validate(&tree, log_stream));
	EXPECT_TRUE(is_unlocked(&tree, log_stream));
	fprintf(log_stream, "\n\n");
}
