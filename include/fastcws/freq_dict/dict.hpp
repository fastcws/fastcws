// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>
#include <cassert>
#include <algorithm>
#include <array>

#include "fastcws/bindings/containers.hpp"
#include "fastcws/aho_corasick.hpp"
#include "fastcws/misc/log2.hpp"
#include "fastcws/rep_aware/string_view.hpp"
#include "fastcws/rep_aware/unique_ptr.hpp"

namespace fastcws {

namespace freq_dict {

template <class Allocator, class IntermediateAllocator, bool UseDAT> struct dict_trie_holder;

template <class Allocator, class IntermediateAllocator>
struct dict_trie_holder<Allocator, IntermediateAllocator, true> {
	using allocator_traits = std::allocator_traits<Allocator>;
	using intermediate_allocator_traits = std::allocator_traits<IntermediateAllocator>;
	using trie_type = aho_corasick::trie<IntermediateAllocator>;
	using trie_allocator = typename intermediate_allocator_traits::template rebind_alloc<trie_type>;
	using double_array_trie_type = aho_corasick::double_array_trie<Allocator>;

	rep_aware::unique_ptr<trie_type, trie_allocator> trie_ = rep_aware::make_unique<trie_type, trie_allocator>(IntermediateAllocator{});
	double_array_trie_type dat_;
	bool finalized_ = false;

	void add_word(std::string_view s) {
		assert(!finalized_); // when using double_array_trie, you cannot add word after finalized
		trie_->add(s);
	}

	void finalize(bool quiet=false) {
		assert(!finalized_); // when using double_array_trie, duplicate call to finalize() is forbidden
		trie_->finalize();
		dat_.build_from(*trie_, quiet);
		trie_ = nullptr;
		finalized_ = true;
	}

	template <class MatchCallback>
	void scan(const std::string_view haystack, MatchCallback matched) const {
		assert(finalized_);
		dat_.scan(haystack, matched);
	}
};

template <class Allocator, class IntermediateAllocator>
struct dict_trie_holder<Allocator, IntermediateAllocator, false> {
	using allocator_traits = std::allocator_traits<Allocator>;
	using trie_type = aho_corasick::trie<Allocator>;

	trie_type trie_;
	bool finalized_ = false;

	void add_word(std::string_view s) {
		trie_.add(s);
		finalized_ = false;
	}

	void finalize(bool quiet=false) {
		(void)quiet;
		trie_.finalize();
		finalized_ = true;
	}

	template <class MatchCallback>
	void scan(const std::string_view haystack, MatchCallback matched) const {
		assert(finalized_);
		trie_.scan(haystack, matched);
	}
};

template <class Allocator = std::allocator<int>, class IntermediateAllocator = Allocator, bool UseDAT = false>
struct dict {
	using allocator_traits = std::allocator_traits<Allocator>;

	using string_view_type = rep_aware::basic_string_view<char, std::char_traits<char>,
		  typename allocator_traits::template rebind_alloc<char>>;

	static constexpr size_t blk_size = 512UL * 1024UL;
	list<std::array<char, blk_size>,
		typename allocator_traits::template rebind_alloc<
			std::array<char, blk_size>>> storage_;
	size_t storage_last_blk_used_ = 0;
	vector<std::pair<string_view_type, uint64_t>,
		typename allocator_traits::template rebind_alloc<
			std::pair<string_view_type, uint64_t>>> freq_;
	uint64_t total_ = 0;

	dict_trie_holder<Allocator, IntermediateAllocator, UseDAT> trie_holder_;

	void add_word(std::string_view word, uint64_t freq) {
		assert(word.size() <= blk_size);
		if (storage_.empty()) {
			storage_.emplace_back();
		}
		if ((storage_last_blk_used_ + word.size()) > blk_size) {
			storage_.emplace_back();
			storage_last_blk_used_ = 0;
		}
		std::copy(word.cbegin(), word.cend(), storage_.back().begin() + storage_last_blk_used_);
		string_view_type sv{storage_.back().data() + storage_last_blk_used_, word.size()};
		storage_last_blk_used_ += word.size();

		freq_.emplace_back(sv, freq);
		trie_holder_.add_word(std::string_view{sv.data(), sv.size()});
		total_ += freq;
	}

	void finalize(bool quiet=false) {
		std::sort(freq_.begin(), freq_.end());
		trie_holder_.finalize(quiet);
	}

	uint64_t get_freq(std::string_view s) const noexcept {
		auto it = std::lower_bound(freq_.cbegin(), freq_.cend(), std::pair<string_view_type, uint64_t>{string_view_type{s.data(), s.size()}, 0});
		return it->second;
	}

	template <class Weight>
	Weight _calc_weight(uint64_t freq) const noexcept {
		return calc_log2<Weight>::log2(total_) - calc_log2<Weight>::log2(freq);
	}

	template<class WordDag>
	void add_edges(WordDag& dag) const {
		using dag_t = WordDag;
		using weight_t = typename dag_t::weight_t;
	
		auto sentence = dag.sentence();
		trie_holder_.scan(sentence, [this, sentence, &dag](size_t end_pos, std::string_view word) {
			dag.add_edge(end_pos - word.size(), end_pos, this->_calc_weight<weight_t>(this->get_freq(word)));
		});
	}

	template<class WordDag>
	typename WordDag::weight_t suggest_single_rune_weight() const noexcept {
		using dag_t = WordDag;
		using weight_t = typename dag_t::weight_t;
		return calc_log2<weight_t>::log2(total_ + 1);
	}

	template<class WordDag>
	typename WordDag::weight_t suggest_hmm_model_weight() const noexcept {
		using dag_t = WordDag;
		using weight_t = typename dag_t::weight_t;
		return 2 * (calc_log2<weight_t>::log2(total_) - calc_log2<weight_t>::log2(std::min<uint64_t>(total_, 2000)));
	}
};

}

}

