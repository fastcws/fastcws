#include "gtest/gtest.h"

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "fastcws/suspendable_region.hpp"
#include "fastcws/bindings/containers.hpp"

#ifndef FASTCWS_NO_BOOST

TEST(managed_region, suspend_and_recover) {
	using fastcws::vector;
	using fastcws::basic_string;

	using namespace fastcws::suspendable_region;
	const uint16_t vec_ptr_tag = 0x87;

	std::stringstream ss;

	{
		managed_region<seats::seat_0> p{4096};
		auto alloc_of = allocator_of(p);
		auto char_alloc = alloc_of.get<char>();
		using region_string = basic_string<
			char,
			std::char_traits<char>,
			decltype(char_alloc)
				>;
		auto string_alloc = alloc_of.get<region_string>();
		using region_vector = vector<
			region_string,
			decltype(string_alloc)
				>;
		auto vec_alloc = alloc_of.get<region_vector>();
		using alloc_traits = std::allocator_traits<decltype(vec_alloc)>;
		auto vec_ptr = alloc_traits::allocate(vec_alloc, 1);
		p.tag_ptr(vec_ptr_tag, vec_ptr);
		alloc_traits::construct(vec_alloc, vec_ptr.get(), string_alloc);

		vec_ptr->emplace_back("hello", char_alloc);
		vec_ptr->emplace_back("world", char_alloc);

		p.suspend(ss);

		vec_ptr->emplace_back("1234");
	}

	{
		char buffer[4096];
		auto p = managed_region<seats::seat_1>::recover(ss, buffer, 4096);
		auto alloc_of = allocator_of(p);
		auto char_alloc = alloc_of.get<char>();
		using region_string = basic_string<
			char,
			std::char_traits<char>,
			decltype(char_alloc)
		>;
		auto string_alloc = alloc_of.get<region_string>();
		using region_vector = vector<
			region_string,
			decltype(string_alloc)
		>;
		auto vec_ptr = p.retrieve_ptr<region_vector>(vec_ptr_tag);

		EXPECT_EQ(vec_ptr->size(), 2);
		EXPECT_EQ((*vec_ptr)[0], "hello");
		EXPECT_EQ((*vec_ptr)[1], "world");
	}
}

#endif

