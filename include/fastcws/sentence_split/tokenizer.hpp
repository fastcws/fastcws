// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <istream>
#include <iterator>
#include <functional>

#include "fastcws/aho_corasick.hpp"
#include "fastcws/sentence_split/dict.hpp"

#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace sentence_split {

template <class Trie>
struct tokenizer {
	using trie_t = Trie;
	using scan_state_t = typename trie_t::scan_state_t;
	using sentence_callback_t = std::function<void(std::string)>;

	vector<char> vec_;
	scan_state_t scan_state_ = trie_t::get_instance().initial_scan_state();
	sentence_callback_t on_sentence_;

	tokenizer(sentence_callback_t on_sentence)
		: on_sentence_(std::move(on_sentence)) {}

	void process(std::string_view sv) {
		size_t processed = 0;
		auto match_cb = [this, &processed, sv](size_t end_pos, std::string_view found){
			(void)found;
			std::copy(
				sv.cbegin() + processed,
				sv.cbegin() + end_pos,
				std::back_inserter(this->vec_)
			);
			if (this->vec_.size()) {
				this->on_sentence_(std::string{this->vec_.data(), this->vec_.size()});
				this->vec_.resize(0);
			}
			processed = end_pos;
		};
		trie_t::get_instance().scan(sv, match_cb, &scan_state_);
		std::copy(
			sv.cbegin() + processed,
			sv.cend(),
			std::back_inserter(vec_)
		);
	}

	void finish() {
		if (vec_.size()) {
			on_sentence_(std::string{vec_.data(), vec_.size()});
			vec_.resize(0);
		}
	}
};

template <class Tokenizer, size_t ChunkSize = 8192>
struct istream_tokenizer {
	using tokenizer_t = Tokenizer;
	tokenizer_t tok_;
	std::istream& is_;
	deque<std::string> q_;

	istream_tokenizer(std::istream& is)
	: tok_([this](std::string sentence){
		this->_on_sentence(std::move(sentence));
	}), is_(is)
	{}

	void _on_sentence(std::string sentence) {
		q_.emplace_back(std::move(sentence));
	}

	void _process_chunk() {
		std::array<char, ChunkSize> buffer;
		(void)is_.peek();
		size_t sz = std::min<size_t>(buffer.size(), is_.rdbuf()->in_avail());
		if (!is_.eof() && sz == 0) { // for impl which doesn't support rdbuf()->in_avail()
			sz = 1;
		}
		is_.read(buffer.data(), sz);
		tok_.process(std::string_view{buffer.data(), sz});
		if (is_.eof()) {
			tok_.finish();
		}
	}

	bool _read_sentence(std::string& s) {
		while (q_.empty() && is_.good()) {
			_process_chunk();
		}
		if (q_.empty()) {
			return false;
		}
		s = q_.front();
		q_.pop_front();
		return true;
	}

	friend bool operator>>(istream_tokenizer& tok, std::string &s) {
		return tok._read_sentence(s);
	}
};

}

using sentence_tokenizer = sentence_split::tokenizer<sentence_split::utf8_sentence_split_trie>;
using istream_sentence_tokenizer = sentence_split::istream_tokenizer<sentence_tokenizer>;

}
