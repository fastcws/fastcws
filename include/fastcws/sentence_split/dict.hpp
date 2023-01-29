// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "fastcws/aho_corasick.hpp"

namespace fastcws {

namespace sentence_split {

struct utf8_sentence_split_trie : aho_corasick::trie<> {
	using base_t = aho_corasick::trie<>;

	utf8_sentence_split_trie() {
		base_t::add(u8"。");
		base_t::add(u8"？");
		base_t::add(u8"！");
		base_t::add("\n");
		base_t::add("\r\n");
		base_t::finalize();
	}

	static const utf8_sentence_split_trie &get_instance() noexcept {
		static utf8_sentence_split_trie trie{};
		return trie;
	}
};

}

}

