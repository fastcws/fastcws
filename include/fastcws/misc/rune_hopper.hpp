// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <cstdint>
#include <cstddef>

#include "fastcws/error.hpp"

namespace fastcws {

namespace rune_hopper {

struct utf8_hopper {
	static size_t hop(uint8_t byte) noexcept {
		if ((byte & 0xe0) == 0xc0) {
			return 2;
		} else if ((byte & 0xf0) == 0xe0) {
			return 3;
		} else if ((byte & 0xf8) == 0xf0) {
			return 4;
		} else {
			return 1;
		}
	}
};

struct utf16_hopper {
	static size_t hop(uint8_t byte) noexcept {
		if ((byte & 0xfc) == 0xd8) {
			return 4;
		} else {
			return 2;
		}
	}
};

struct utf32_hopper {
	static size_t hop(uint8_t byte) noexcept {
		(void)byte;
		return 4;
	}
};

}

// TODO what to do with um, graphemes?

template <class RuneHopper = rune_hopper::utf8_hopper, class WordDag>
void populate_rune_chain(WordDag& dag, typename WordDag::weight_t w) {
	for (size_t i = 0; i < dag.sentence().size();) {
		size_t hop_over = RuneHopper::hop(dag.sentence()[i]);
		if ((i + hop_over) > dag.sentence().size()) {
			throw exception::bad_encoding{};
		}
		dag.add_edge(i, i + hop_over, w);
		i += hop_over;
	}
}

template <class RuneHopper = rune_hopper::utf8_hopper, class StringViewOutputIterator, class StringView>
void split_runes(StringView sentence, StringViewOutputIterator out) {
	for (size_t i = 0; i < sentence.size();) {
		size_t hop_over = RuneHopper::hop(sentence[i]);
		if ((i + hop_over) > sentence.size()) {
			throw exception::bad_encoding{};
		}
		*out = sentence.substr(i, hop_over);
		i += hop_over;
		out++;
	}
}

}

