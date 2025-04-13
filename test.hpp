extern "C" {
#include "insert.h"
#include "io.h"
#include "memory.h"
#include "node.h"
#include "search.h"
#include "validate.h"
};

#include <gtest/gtest.h>


FILE *log_stream = fopen("log.txt", "w");


TEST(InitTest, Tree) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	mem_reset_all();

	for (bptr_t i = 0; i < MAX_LEVELS * MAX_NODES_PER_LEVEL; ++i) {
		Node n = mem_read(i);
		for (li_t j = 0; j < TREE_ORDER; ++j) {
			EXPECT_EQ(n.keys[j], INVALID);
		}
	}

	EXPECT_TRUE(is_unlocked(root, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(ValidateTest, RootOneChild) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	AddrNode root;
	root.addr = MAX_LEAVES;
	mem_reset_all();
	root.node = mem_read_lock(root.addr);
	AddrNode lchild = {.node = mem_read_lock(0), .addr = 0};

	root.node.keys[0] = 6; root.node.values[0].ptr = 0;
	lchild.node.keys[0] = 1; lchild.node.values[0].data = -1;
	lchild.node.keys[1] = 2; lchild.node.values[1].data = -2;
	lchild.node.keys[2] = 4; lchild.node.values[2].data = -4;
	lchild.node.keys[3] = 5; lchild.node.values[3].data = -5;
	mem_write_unlock(&root);
	mem_write_unlock(&lchild);
	dump_node_list(log_stream, root.addr);

	EXPECT_FALSE(validate(root.addr, log_stream));
	EXPECT_TRUE(is_unlocked(root.addr, log_stream));
	fprintf(log_stream, "\n\n");
}


#if TREE_ORDER >= 4
TEST(SearchTest, RootIsLeaf) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	AddrNode root;
	root.addr = 0;
	mem_reset_all();
	root.node = mem_read_lock(root.addr);
	bstatusval_t result;

	root.node.keys[0] = 1; root.node.values[0].data = -1;
	root.node.keys[1] = 2; root.node.values[1].data = -2;
	root.node.keys[2] = 4; root.node.values[2].data = -4;
	root.node.keys[3] = 5; root.node.values[3].data = -5;
	mem_write_unlock(&root);
	dump_node_list(log_stream, root.addr);
	EXPECT_EQ(search(root.addr, 0).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 3).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 6).status, NOT_FOUND);
	result = search(root.addr, 1);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(root.addr, 2);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(root.addr, 4);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(root.addr, 5);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);

	EXPECT_TRUE(validate(root.addr, log_stream));
	EXPECT_TRUE(is_unlocked(root.addr, log_stream));
	fprintf(log_stream, "\n\n");
}
#endif

#if TREE_ORDER == 4
TEST(SearchTest, OneInternal) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	AddrNode root;
	root.addr = MAX_LEAVES;
	mem_reset_all();
	root.node = mem_read_lock(root.addr);
	AddrNode lchild = {.node = mem_read_lock(0), .addr = 0};
	AddrNode rchild = {.node = mem_read_lock(1), .addr = 1};
	bstatusval_t result;

	root.node.keys[0] = 6; root.node.values[0].ptr = 0;
	root.node.keys[1] = 12; root.node.values[1].ptr = 1;
	lchild.node.keys[0] = 1; lchild.node.values[0].data = -1;
	lchild.node.keys[1] = 2; lchild.node.values[1].data = -2;
	lchild.node.keys[2] = 4; lchild.node.values[2].data = -4;
	lchild.node.keys[3] = 5; lchild.node.values[3].data = -5;
	lchild.node.next = 1;
	rchild.node.keys[0] = 7; rchild.node.values[0].data = -7;
	rchild.node.keys[1] = 8; rchild.node.values[1].data = -8;
	rchild.node.keys[2] = 10; rchild.node.values[2].data = -10;
	rchild.node.keys[3] = 11; rchild.node.values[3].data = -11;
	mem_write_unlock(&root);
	mem_write_unlock(&lchild);
	mem_write_unlock(&rchild);
	dump_node_list(log_stream, root.addr);
	EXPECT_EQ(search(root.addr, 0).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 3).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 6).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 9).status, NOT_FOUND);
	EXPECT_EQ(search(root.addr, 12).status, NOT_FOUND);
	result = search(root.addr, 1);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -1);
	result = search(root.addr, 2);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -2);
	result = search(root.addr, 4);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -4);
	result = search(root.addr, 5);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -5);
	result = search(root.addr, 7);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -7);
	result = search(root.addr, 8);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -8);
	result = search(root.addr, 10);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -10);
	result = search(root.addr, 11);
	EXPECT_EQ(result.status, SUCCESS);
	EXPECT_EQ(result.value.data, -11);

	EXPECT_TRUE(validate(root.addr, log_stream));
	EXPECT_TRUE(is_unlocked(root.addr, log_stream));
	fprintf(log_stream, "\n\n");
}
#endif


TEST(InsertTest, LeafNode) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	mem_reset_all();
	bval_t value;

	value.data = 2;
	EXPECT_EQ(insert(&root, 0, value), SUCCESS);
	EXPECT_EQ(mem_read(root).keys[0], 0);
	EXPECT_EQ(mem_read(root).values[0].data, 2);
	dump_node_list(log_stream, root);
	ASSERT_TRUE(is_unlocked(root, log_stream));

	value.data = 3;
	EXPECT_EQ(insert(&root, 5, value), SUCCESS);
	EXPECT_EQ(mem_read(root).keys[1], 5);
	EXPECT_EQ(mem_read(root).values[1].data, 3);
	dump_node_list(log_stream, root);
	ASSERT_TRUE(is_unlocked(root, log_stream));

	value.data = 1;
	EXPECT_EQ(insert(&root, 3, value), SUCCESS);
	EXPECT_EQ(mem_read(root).keys[1], 3);
	EXPECT_EQ(mem_read(root).values[1].data, 1);
	EXPECT_EQ(mem_read(root).keys[2], 5);
	EXPECT_EQ(mem_read(root).values[2].data, 3);
	dump_node_list(log_stream, root);
	ASSERT_TRUE(is_unlocked(root, log_stream));

	EXPECT_TRUE(validate(root, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, SplitRoot) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	bptr_t lchild;
	bval_t value;
	mem_reset_all();

	value.data = 0;
	EXPECT_EQ(insert(&root, -value.data, value), SUCCESS);
	dump_node_list(log_stream, root);
	EXPECT_EQ(mem_read(root).keys[0], -value.data);
	EXPECT_EQ(mem_read(root).values[0].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream));
	value.data = -5;
	EXPECT_EQ(insert(&root, -value.data, value), SUCCESS);
	dump_node_list(log_stream, root);
	EXPECT_EQ(mem_read(root).keys[1], -value.data);
	EXPECT_EQ(mem_read(root).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream));
	value.data = -3;
	EXPECT_EQ(insert(&root, -value.data, value), SUCCESS);
	dump_node_list(log_stream, root);
	EXPECT_EQ(mem_read(root).keys[1], -value.data);
	EXPECT_EQ(mem_read(root).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream));
	// This one causes a split
	value.data = -1;
	EXPECT_EQ(insert(&root, -value.data, value), SUCCESS);
	dump_node_list(log_stream, root);
	lchild = mem_read(root).values[0].ptr;
	EXPECT_EQ(mem_read(lchild).keys[1], -value.data);
	EXPECT_EQ(mem_read(lchild).values[1].data, value.data);
	ASSERT_TRUE(is_unlocked(root, log_stream));
	value.data = -4;
	EXPECT_EQ(insert(&root, -value.data, value), SUCCESS);
	dump_node_list(log_stream, root);
	ASSERT_TRUE(is_unlocked(root, log_stream));
	ASSERT_TRUE(is_unlocked(lchild, log_stream));

	EXPECT_TRUE(validate(root, log_stream));
	fprintf(log_stream, "\n\n");
}

TEST(InsertTest, InsertUntilItBreaks) {
	const testing::TestInfo* const test_info =
		testing::UnitTest::GetInstance()->current_test_info();
	fprintf(log_stream, "=== %s.%s ===\n",
		test_info->test_suite_name(), test_info->name()
	);

	bptr_t root = 0;
	bval_t value;
	mem_reset_all();

	// Insert values
	for (uint_fast8_t i = 1; i <= (TREE_ORDER/2)*(MAX_LEAVES+1); ++i) {
		value.data = -i;
		ASSERT_EQ(insert(&root, i, value), SUCCESS);
		dump_node_list(log_stream, root);
		ASSERT_TRUE(is_unlocked(root, log_stream));
	}
	// Check that they're instantiated in memory correctly
	uint_fast8_t next = 1;
	for (bptr_t i = 0; i < MAX_LEAVES; ++i) {
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

	EXPECT_TRUE(validate(root, log_stream));
	fprintf(log_stream, "\n\n");
}
