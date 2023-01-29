#include "gtest/gtest.h"

#include <string>
#include <iostream>
#include <fstream>

#include "fastcws/word_dag.hpp"

TEST(word_dag, kahn) {
	using namespace fastcws;

	word_dag::dag<> dag{"012345"};
	dag.add_edge(0, 2, 5.0);
	dag.add_edge(2, 5, 10.0);
	dag.add_edge(0, 1, 7.0);
	dag.add_edge(1, 5, 9.0);
	dag.add_edge(5, 6, 4.0);

	auto result = kahn(dag);

	EXPECT_EQ(result.path.size(), 2);
	EXPECT_EQ(result.path[0], 2);
	EXPECT_EQ(result.path[1], 5);
	EXPECT_EQ(result.score, 19.0);
}

