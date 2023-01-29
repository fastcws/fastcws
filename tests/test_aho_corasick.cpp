#include "gtest/gtest.h"

#include <string>
#include <iostream>
#include <vector>
#include <tuple>

#include "fastcws/aho_corasick.hpp"

TEST(aho_corasick, scan) {
	using namespace fastcws;

	aho_corasick::trie trie;
	trie.add("i");
	trie.add("he");
	trie.add("his");
	trie.add("she");
	trie.add("hers");
	trie.finalize();

	std::string to_scan = "ushersheishis";
	std::vector<std::tuple<size_t, size_t>> matches;
	auto matched = [to_scan, &matches](size_t end_pos, std::string_view found){
		matches.emplace_back(end_pos - found.size(), found.size());
	};
	trie.scan(to_scan, matched);

	std::vector<std::tuple<size_t, size_t>> expected_matches;
	expected_matches.emplace_back(1, 3);
	expected_matches.emplace_back(2, 2);
	expected_matches.emplace_back(2, 4);
	expected_matches.emplace_back(5, 3);
	expected_matches.emplace_back(6, 2);
	expected_matches.emplace_back(8, 1);
	expected_matches.emplace_back(11, 1);
	expected_matches.emplace_back(10, 3);

	EXPECT_EQ(matches, expected_matches);
}

TEST(double_array_trie, build) {
	using namespace fastcws;

	aho_corasick::trie trie;
	trie.add("i");
	trie.add("he");
	trie.add("his");
	trie.add("she");
	trie.add("hers");
	//  TODO trie.add("asdfghjkl"); // test merging
	trie.finalize();

	aho_corasick::double_array_trie dat;
	dat.build_from(trie);


	std::string to_scan = "ushersheishis";
	std::vector<std::tuple<size_t, size_t>> matches;
	auto matched = [to_scan, &matches](size_t end_pos, std::string_view found){
		matches.emplace_back(end_pos - found.size(), found.size());
	};
	dat.scan(to_scan, matched);

	std::vector<std::tuple<size_t, size_t>> expected_matches;
	expected_matches.emplace_back(1, 3);
	expected_matches.emplace_back(2, 2);
	expected_matches.emplace_back(2, 4);
	expected_matches.emplace_back(5, 3);
	expected_matches.emplace_back(6, 2);
	expected_matches.emplace_back(8, 1);
	expected_matches.emplace_back(11, 1);
	expected_matches.emplace_back(10, 3);

	EXPECT_EQ(matches, expected_matches);
	/**
	size_t i = 0;
	constexpr auto not_used = aho_corasick::double_array_trie::unit_type::not_used;
	for (const auto& u : dat.units_) {
		std::cout << i << ": ";
		if (!u.used()) {
			std::cout << "<not used>\n";
		} else {
			if (u.base == not_used) {
				std::cout << "base=<>";
			} else {
				std::cout << "base=" << u.base;
			}
			if (u.check == not_used) {
				std::cout << " check=<>";
			} else {
				std::cout << " check=" << u.check;
			}
			std::cout << " fail=" << u.fail
				<< " tail='" << dat.tails_[u.tail].match
				<< "'[-" << dat.tails_[u.tail].tail_size << "]\n";
		}
		i++;
	}
	**/
}

