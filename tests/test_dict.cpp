#include "gtest/gtest.h"

#include <string>
#include <iostream>
#include <fstream>

#include "fastcws/freq_dict.hpp"
#include "fastcws/word_dag.hpp"

TEST(dict, load_and_build_dag) {
	using namespace fastcws;

	freq_dict::dict d{};
	d.add_word("雪花", 10);
	d.add_word("最终", 10);
	d.add_word("果实", 10);
	d.finalize();

	word_dag::dag<> dag{"而雪花是最终的果实"};
	d.add_edges(dag);

	EXPECT_EQ(dag.adjacents(3).count(9), 1);
	EXPECT_EQ(dag.adjacents(12).count(18), 1);
	EXPECT_EQ(dag.adjacents(21).count(27), 1);
}

