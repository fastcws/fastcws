// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <cstdint>
#include <cstddef>

#include "fastcws/misc/rune_hopper.hpp"

namespace fastcws {

enum class special_t {
	not_special, // is_chinese() == true
	whitespace, // space, tab
	crlf, // '\r' & '\n'
	cn_dash, // '——' chinese dash, commonly bigram
	cn_ellipsis, // ……' chinese ellipsis, commonly bigram
	cn_taitou, // U+3000 ideographic space
	others // all others will be skipped altogether (numerics, english letters, etc.)
};

namespace special_hopper {

struct utf8_special_hopper {
	using rune_hopper_t = rune_hopper::utf8_hopper;

	static bool is_chinese(std::string_view sv) noexcept {
		if (sv.size() != 3) {
			return false;
		}
		return (sv >= u8"一") && (sv <= u8"龥");
	}

	static special_t classify_special(std::string_view rune) noexcept {
		if (is_chinese(rune)) {
			return special_t::not_special;
		}
		if (rune.size() == 1) {
			switch (rune[0]) {
				case ' ':
				case '\t':
				return special_t::whitespace;
				case '\r':
				case '\n':
				return special_t::crlf;
				default:;
			}
		}
		if (rune == u8"—") {
			return special_t::cn_dash;
		} else if (rune == u8"…") {
			return special_t::cn_ellipsis;
		} else if (rune == u8"　") {
			return special_t::cn_taitou;
		}
		return special_t::others;
	}
};

}

template <class SpecialHopper=special_hopper::utf8_special_hopper, class WordDag>
void add_special_edges(WordDag &dag) {
	vector<std::string_view> runes;
	split_runes<typename SpecialHopper::rune_hopper_t>(dag.sentence(), std::back_inserter(runes));

	size_t same_class_len = 0;
	size_t pos = 0;
	special_t sp = special_t::not_special;
	for (auto rune : runes) {
		special_t curr_sp = SpecialHopper::classify_special(rune);
		if (curr_sp != sp) {
			if (sp != special_t::not_special) {
				dag.add_edge(pos - same_class_len, pos, 0);
			}
			sp = curr_sp;
			same_class_len = rune.size();
		} else {
			same_class_len += rune.size();
		}
		pos += rune.size();
	}
	if (sp != special_t::not_special) {
		dag.add_edge(pos - same_class_len, pos, 0);
	}
}

}

