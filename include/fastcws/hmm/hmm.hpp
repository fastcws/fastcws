// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <memory>
#include <array>
#include <ostream>
#include <cassert>
#include <algorithm>

#include "fastcws/hmm/normalizer.hpp"
#include "fastcws/misc/log2.hpp"
#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace hmm {

template <class T, class ObserveType, size_t NumStates, class Allocator>
struct _model_params{
	static constexpr size_t num_states_ = NumStates;
	using observe_t = ObserveType;
	using element_t = T;

	using allocator_traits = std::allocator_traits<Allocator>;

	std::array<element_t, num_states_> pi = {0};
	std::array<std::array<element_t, num_states_>, num_states_> a = {0};
	unordered_map<
		observe_t,
		std::array<element_t, num_states_>,
		std::hash<observe_t>,
		std::equal_to<observe_t>,
		typename allocator_traits::template rebind_alloc<
			std::pair <
				const observe_t,
				std::array<element_t, num_states_>
			>
		>
	> b{};

	void dump(std::ostream& os) const {
		os << "pi: [ ";
		for (auto &ele : pi) {
			os << ele << ", ";
		}
		os << "]\n"
			<< "a: [\n";
		for (auto& row : a) {
			os << "[ ";
			for (auto& ele : row) {
				os << ele << ", ";
			}
			os << "] ,\n";
		}
		os << "]\n"
			<< "b: [\n";
		for (auto& [k, row] : b) {
			os << k << " : " << "[ ";
			for (auto& ele : row) {
				os << ele << ", ";
			}
			os << "] ,\n";
		}
		os << "]\n";
	}
};

// hmm model with dynamic number of observables -- suitable for chinese segmentation workloads
// StateEnum values must be [0, 1, .., NumStates)
template <class ObserveType,
		  class StateEnum,
		  size_t NumStates,
		  class Normalizer = basic_normalizer<uint64_t, double>,
		  class Allocator = std::allocator<int> // this is always rebinded
		  >
struct model {
	using observe_t = ObserveType;
	using state_enum_t = StateEnum;
	using normalizer_t = Normalizer;

	using freq_t = typename normalizer_t::input_t;
	using relative_freq_t = typename normalizer_t::output_t;

	static constexpr size_t num_states_ = NumStates;

	_model_params<freq_t, observe_t, num_states_, Allocator> training_;
	_model_params<relative_freq_t, observe_t, num_states_, Allocator> normalized_;
	
	template <class ObserveTypeIterator, class StateEnumIterator> /* TODO concepts */
	void train(const ObserveTypeIterator& x_begin, const ObserveTypeIterator& x_end,
			const StateEnumIterator& y_begin, const StateEnumIterator& y_end) {
		assert((x_end - x_begin) == (y_end - y_begin));
		if (x_end == x_begin) {
			return;
		}

		training_.pi[static_cast<size_t>(*y_begin)]++;
		auto x_it = x_begin;
		auto y_it = y_begin;
		while (x_it != x_end) {
			training_.b[*x_it][static_cast<size_t>(*y_it)]++;
			x_it++;
			y_it++;
		}

		if ((x_end - x_begin) < 2) {
			return;
		}
		auto y_from_it = y_begin;
		auto y_to_it = y_begin + 1;
		while (y_to_it != y_end) {
			training_.a[static_cast<size_t>(*y_from_it)][static_cast<size_t>(*y_to_it)]++;
			y_from_it++;
			y_to_it++;
		}
	}

	void normalize() {
		Normalizer::normalize(training_.pi.cbegin(), training_.pi.cend(), normalized_.pi.begin());
		for (size_t i = 0; i < num_states_; i++) {
			Normalizer::normalize(training_.a[i].cbegin(), training_.a[i].cend(), normalized_.a[i].begin());
		}
		for (auto [obs, emit] : training_.b) {
			Normalizer::normalize(emit.cbegin(), emit.cend(), normalized_.b[obs].begin());
		}
	}

	void dump(std::ostream& os) const {
		os << "training: \n";
		training_.dump(os);
		os << "normalized: \n";
		normalized_.dump(os);
	}

	relative_freq_t _get_b(observe_t obs, state_enum_t state) const noexcept {
		if (normalized_.b.count(obs)) {
			return normalized_.b.at(obs)[static_cast<size_t>(state)];
		} else {
			return -calc_log2<relative_freq_t>::log2(num_states_);
		}
	}

	relative_freq_t initial(state_enum_t state, observe_t observed) const noexcept {
		return normalized_.pi[static_cast<size_t>(state)] + _get_b(observed, state);
	}

	relative_freq_t calc_p(state_enum_t from_state, state_enum_t to_state, observe_t to_observed) const noexcept {
		return _get_b(to_observed, to_state) + normalized_.a[static_cast<size_t>(from_state)][static_cast<size_t>(to_state)];
	}

	bool trival() const noexcept {
		if (std::accumulate(training_.pi.cbegin(), training_.pi.cend(), freq_t(0)) == 0) {
			return true;
		}
		for (const auto& row : training_.a) {
			if (std::accumulate(row.cbegin(), row.cend(), freq_t(0)) == 0) {
				return true;
			}
		}
		return false;
	}
};

}

}

