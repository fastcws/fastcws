// SPDX-License-Identifier: BSD-2-Clause

#include <string_view>
#include <cassert>
#include <fstream>
#include <iostream>

#include "fastcws/freq_dict.hpp"
#include "fastcws/hmm.hpp"
#include "fastcws/suspendable_region.hpp"
#include "fastcws_defaults/definitions.hpp"
#include "fastcws_defaults/compressor.hpp"

inline constexpr size_t max_snapshot_size = 2UL * 1024UL * 1024UL * 1024UL;
using fastcws::defaults::seat_dict;
using fastcws::defaults::seat_model;
using fastcws::defaults::dict_ptr_tag;
using fastcws::defaults::model_ptr_tag;

void save_dict_snapshot(const char* dict_filename, const char* snapshot_filename) {
	namespace suspendable_region = fastcws::suspendable_region;
	using fastcws::suspendable_region::managed_region;

	managed_region<seat_dict> reg{max_snapshot_size};
	using int_allocator = suspendable_region::allocator<int, decltype(reg)>;
	using dict_type = fastcws::freq_dict::dict<int_allocator, std::allocator<int>, true>;
	auto dict_alloc = suspendable_region::allocator_of(reg).get<dict_type>();
	using alloc_traits = std::allocator_traits<decltype(dict_alloc)>;

	auto dict_ptr = alloc_traits::allocate(dict_alloc, 1);
	reg.tag_ptr(dict_ptr_tag, dict_ptr);
	alloc_traits::construct(dict_alloc, dict_ptr.get());
	{
		std::ifstream ifs{dict_filename};
		assert(ifs.good());
		load_dict(ifs, *dict_ptr, false);
	}
	{
		std::ofstream ofs{snapshot_filename, std::ios::binary};
		assert(ofs.good());
		reg.suspend(ofs);
	}
}

void dump_dict_snapshot(const char *snapshot_filename) {
	namespace suspendable_region = fastcws::suspendable_region;
	using fastcws::suspendable_region::managed_region;

	std::ifstream ifs{snapshot_filename, std::ios::binary};
	assert(ifs.good());
	auto reg = managed_region<seat_dict>::recover(ifs);
	ifs.close();
	using int_allocator = suspendable_region::allocator<int, decltype(reg)>;
	using dict_type = fastcws::freq_dict::dict<int_allocator, std::allocator<int>, true>;

	auto dict_ptr = reg.retrieve_ptr<dict_type>(dict_ptr_tag);
	save_dict(*dict_ptr, std::cout);
	std::flush(std::cout);
}

void save_hmm_model_snapshot(const char* model_filename, const char* snapshot_filename) {
	namespace suspendable_region = fastcws::suspendable_region;
	using fastcws::suspendable_region::managed_region;

	managed_region<seat_model> reg{max_snapshot_size};
	using int_allocator = suspendable_region::allocator<int, decltype(reg)>;
	using model_type = fastcws::hmm::wseg_4tag::model<fastcws::hmm::basic_normalizer<uint64_t, double>, int_allocator>;
	auto model_alloc = suspendable_region::allocator_of(reg).get<model_type>();
	using alloc_traits = std::allocator_traits<decltype(model_alloc)>;

	auto model_ptr = alloc_traits::allocate(model_alloc, 1);
	reg.tag_ptr(model_ptr_tag, model_ptr);
	alloc_traits::construct(model_alloc, model_ptr.get());
	{
		std::ifstream ifs{model_filename};
		assert(ifs.good());
		*model_ptr = fastcws::hmm::wseg_4tag::load<model_type>(ifs);
	}
	{
		std::ofstream ofs{snapshot_filename, std::ios::binary};
		assert(ofs.good());
		reg.suspend(ofs);
	}
}

void snapshot2hpp(const char * identifier, const char* snapshot_filename, const char* output_filename) {
	std::ifstream ifs{snapshot_filename, std::ios::in | std::ios::binary};
	assert(ifs.good());
	std::vector<char> raw_data{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
	ifs.close();

	std::vector<char> compressed_data;
	compressed_data.resize(raw_data.size() + 4096);

	size_t sz = fastcws::defaults::compressor::compress(raw_data.data(), raw_data.size(), compressed_data.data(), compressed_data.size());
	compressed_data.resize(sz);

	std::ofstream ofs{output_filename};
	assert(ofs.good());

	std::ostringstream oss;
	oss << "__" << identifier << "_data";
	std::string array_name = oss.str();

	oss.str("");
	oss << "compressed_" << identifier << "_data_begin";
	std::string compressed_data_begin_name = oss.str();

	oss.str("");
	oss << "compressed_" << identifier << "_data_size";
	std::string compressed_data_size_name = oss.str();

	oss.str("");
	oss << "decompressed_" << identifier << "_data_size";
	std::string decompressed_data_size_name = oss.str();

	ofs << "#pragma once\n"
		<< "\n"
		<< "#include <cstdint>\n"
		<< "#include <cstddef>\n"
		<< "\n"
		<< "namespace fastcws {\n"
		<< "namespace defaults {\n"
		<< "\n"
		<< "inline const uint8_t " << array_name << "[] = {\n";

	size_t i = 0;
	for (char ch : compressed_data) {
		int ch_i = static_cast<int>(static_cast<uint8_t>(ch));
		if (i == 0) {
			ofs << "  ";
		} else {
			ofs << ", ";
		}
		ofs << "0x" << std::hex << ch_i;
		i++;
		if (i % 16 == 0) {
			ofs << " // " << std::dec << i << "\n";
		}
	}
	ofs << "\n"
		<< "};\n"
		<< "\n"
		<< "inline const uint8_t *const " << compressed_data_begin_name << " = " << std::dec << array_name << ";\n"
		<< "inline const size_t " << compressed_data_size_name << " = " << std::dec << compressed_data.size() << ";\n"
		<< "inline const size_t " << decompressed_data_size_name << " = " << std::dec << raw_data.size() << ";\n"
		<< "\n"
		<< "}\n"
		<< "}\n";
}

int main(int argc, char** argv) {
	assert(argc > 1);
	std::string_view command{argv[1]};
	if (command == "save_dict_snapshot") {
		assert(argc > 3);
		save_dict_snapshot(argv[2], argv[3]);
	} else if (command == "dump_dict_snapshot") {
		assert(argc > 2);
		dump_dict_snapshot(argv[2]);
	} else if (command == "save_hmm_model_snapshot") {
		assert(argc > 3);
		save_hmm_model_snapshot(argv[2], argv[3]);
	} else if (command == "snapshot2hpp") {
		assert(argc > 4);
		snapshot2hpp(argv[2], argv[3], argv[4]);
	} else {
		assert(0);
	}
	return 0;
}
