#include <gtest/gtest.h>

TEST(InsertTest, LeafNode) {
	LeafNode leaf = new_leaf();

	ASSERT_EQ(insert(&leaf, 0, 2), 0);
	ASSERT_EQ(leaf.keys[0], 0);
	ASSERT_EQ(leaf.data[0], 2);

	ASSERT_EQ(insert(&leaf, 5, 3), 0);
	ASSERT_EQ(leaf.keys[1], 5);
	ASSERT_EQ(leaf.data[1], 3);

	ASSERT_EQ(insert(&leaf, 3, 1), 0);
	ASSERT_EQ(leaf.keys[1], 3);
	ASSERT_EQ(leaf.data[1], 1);
	ASSERT_EQ(leaf.keys[2], 5);
	ASSERT_EQ(leaf.data[2], 3);
}
