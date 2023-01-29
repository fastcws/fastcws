// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <algorithm>
#include <string_view>
#include <memory>
#include <ostream>
#include <istream>
#include <sstream>
#include <set>

#include "fastcws/hmm/hmm.hpp"
#include "fastcws/hmm/viterbi.hpp"
#include "fastcws/misc/rune_hopper.hpp"
#include "fastcws/rep_aware/string_view.hpp"
#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace hmm {

namespace wseg_4tag {

enum class state {
	B = 0,
	M = 1,
	E = 2,
	S = 3,
};

static constexpr size_t num_states = 4;

template <class Normalizer = basic_normalizer<uint64_t, double>, class Allocator = std::allocator<int>>
struct model : hmm::model<
	rep_aware::basic_string_view<
		char,
		std::char_traits<char>,
		typename std::allocator_traits<Allocator>::template rebind_alloc<char>
	>,
	state,
	num_states,
	Normalizer,
	Allocator> {
	using allocator_traits = std::allocator_traits<Allocator>;

	using string_view_type = rep_aware::basic_string_view<
		char,
		std::char_traits<char>,
		typename allocator_traits::template rebind_alloc<char>>;
	using string_type = basic_string<
		char,
		std::char_traits<char>,
		typename allocator_traits::template rebind_alloc<char>>;

	using base_t = hmm::model<string_view_type, state, num_states, Normalizer, Allocator>;
	using typename base_t::observe_t;
	using typename base_t::state_enum_t;
	using typename base_t::normalizer_t;

	using typename base_t::freq_t;
	using typename base_t::relative_freq_t;

	static constexpr size_t num_states_ = base_t::num_states_;

	set<string_type, std::less<>,
		typename allocator_traits::template rebind_alloc<string_type>> characters_;

	// stores undelying string in characters_
	template <class ObserveTypeIterator, class StateEnumIterator> /* TODO concepts */
	void train(const ObserveTypeIterator& x_begin, const ObserveTypeIterator& x_end,
			const StateEnumIterator& y_begin, const StateEnumIterator& y_end) {
		vector<string_view_type> real_x;
		for (auto it = x_begin; it != x_end; it++) {
			auto [s_it, inserted] = characters_.insert(string_type{*it});
			(void)inserted;
			real_x.push_back(string_view_type{s_it->data(), s_it->size()});
		}
		base_t::train(real_x.begin(), real_x.end(), y_begin, y_end);
	}

	template<class WordDag, class RuneHopper>
	void add_edges(WordDag& dag, typename WordDag::weight_t weight) const {
		if (this->trival()) {
			return;
		}
		vector<string_view_type> runes;
		split_runes<RuneHopper>(string_view_type{dag.sentence().data(), dag.sentence().size()}, std::back_inserter(runes));

		vector<hmm::wseg_4tag::state> states;
		states.resize(runes.size());
		viterbi(*this, runes.begin(), runes.end(), states.begin());

		size_t edge_start = 0;
		size_t edge_end = 0;
		for (size_t i = 0; i < states.size(); i++) {
			edge_end += runes[i].size();
			if ((states[i] == state::S) || (states[i] == state::E)) {
				dag.add_edge(edge_start, edge_end, weight);
				edge_start = edge_end;
			}
		}
	}
};

template <class Model>
void save(const Model& model, std::ostream& os) {
	os.exceptions(std::ios_base::badbit);

	auto save_joined = [](auto iterable, std::ostream& os) {
		for (auto it = iterable.cbegin(); it != iterable.cend(); ++it) {
			if (it == iterable.cbegin()) {
				os << *it;
			} else {
				os << " " << *it;
			}
		}
		os << "\n";
	};
	save_joined(model.training_.pi, os);
	for (auto& row : model.training_.a) {
		save_joined(row, os);
	}
	for (auto& [k, row] : model.training_.b) {
		os << k << "\n";
		save_joined(row, os);
	}
}

template <class Model = model<>>
Model load(std::istream& is) {
	is.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);

	Model model;
	std::string pi_s;
	std::getline(is, pi_s, '\n');
	std::istringstream pi_ss{pi_s};
	for (auto &ele : model.training_.pi) {
		pi_ss >> ele;
	}
	for (auto& row : model.training_.a) {
		std::string a_line_s;
		std::getline(is, a_line_s, '\n');
		std::istringstream a_line_ss{a_line_s};
		for (auto& ele : row) {
			a_line_ss >> ele;
		}
	}
	is.exceptions(std::ios_base::badbit);
	std::string k_s;
	std::string k_line_s;
	while (std::getline(is, k_s, '\n'), std::getline(is, k_line_s, '\n')) {
		auto [s_it, inserted] = model.characters_.insert(typename Model::string_type{k_s.data(), k_s.size()});
		(void)inserted;
		typename Model::string_view_type k{s_it->data(), s_it->size()};
		std::istringstream k_line_ss{k_line_s};
		for (auto& ele : model.training_.b[k]) {
			k_line_ss >> ele;
		}
	}
	model.normalize();
	return model;
}

}

}

}

