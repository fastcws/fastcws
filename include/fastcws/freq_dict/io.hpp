// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <istream>
#include <sstream>
#include <cassert>

#include "fastcws/freq_dict/dict.hpp"

namespace fastcws {

namespace freq_dict {

template <class Dict>
inline void load_dict(std::istream& is, Dict& dict, bool quiet=true) {
	is.exceptions(std::ios_base::badbit);
	if (!quiet) {
		std::cout << "loading words.." << std::endl;
	}
	std::string word, freq_s;
	while (std::getline(is, word, ' '), std::getline(is, freq_s, '\n')) {
		std::istringstream freq_ss{freq_s};
		freq_ss.exceptions(std::ios_base::badbit | std::ios_base::failbit);
		uint64_t freq;
		freq_ss >> freq;
		dict.add_word(word, freq);
	}
	if (!quiet) {
		std::cout << "finalizing.." << std::endl;
	}
	dict.finalize(quiet);
}

template <class Dict = dict<>>
inline Dict load_dict(std::istream& is, bool quiet=true) {
	Dict ret;
	load_dict(is, ret, quiet);
	return ret;
}

template <class Dict>
inline void save_dict(const Dict& d, std::ostream& os) {
	os.exceptions(std::ios_base::badbit);
	for (const auto& [word, freq] : d.freq_) {
		os << word << " " << freq << "\n";
	}
}

}

}

