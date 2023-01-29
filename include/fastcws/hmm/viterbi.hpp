// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <array>
#include <algorithm>
#include <utility>

#include "fastcws/bindings/containers.hpp"

namespace fastcws {

namespace hmm {

template <class HMMModel, class ObservableIterator, class StateOutputIterator> /* TODO concepts */
void viterbi(const HMMModel& model,
		const ObservableIterator& obs_begin,
		const ObservableIterator& obs_end,
		StateOutputIterator out) {
	using model_t = HMMModel;
	using prob_t = typename model_t::relative_freq_t;
	using state_enum_t = typename model_t::state_enum_t;
	constexpr size_t num_states = model_t::num_states_;

	if (obs_begin == obs_end) {
		return;
	}

	std::array<prob_t, num_states> probs_a;
	std::array<prob_t, num_states> probs_b;
	std::array<prob_t, num_states>* curr_probs = &probs_a;
	std::array<prob_t, num_states>* next_probs = &probs_b;
	vector<std::array<state_enum_t, num_states>> from_state;
	auto it = obs_begin;
	for (size_t i = 0; i < num_states; i++) {
		state_enum_t state = static_cast<state_enum_t>(i);
		(*curr_probs)[i] = model.initial(state, *it);
	}
	it++;
	while (it != obs_end) {
		std::array<state_enum_t, num_states> curr_from_state;
		for (size_t i = 0; i < num_states; i++) {
			for (size_t j = 0; j < num_states; j++) {
				state_enum_t from_state = static_cast<state_enum_t>(i);
				state_enum_t to_state = static_cast<state_enum_t>(j);

				prob_t p = (*curr_probs)[i] + model.calc_p(from_state, to_state, *it);
				if ((i == 0) || (p > (*next_probs)[j])) {
					curr_from_state[j] = from_state;
					(*next_probs)[j] = p;
				}
			}
		}
		std::swap(curr_probs, next_probs);
		from_state.push_back(curr_from_state);
		it++;
	}
	out += from_state.size();
	auto max_prob_it = std::max_element((*curr_probs).begin(), (*curr_probs).end());
	size_t state_idx = max_prob_it - (*curr_probs).begin();
	*out = static_cast<state_enum_t>(state_idx);
	for (auto from_state_it = from_state.rbegin(); from_state_it != from_state.rend(); from_state_it++) {
		out--;
		state_idx = static_cast<size_t>((*from_state_it)[state_idx]);
		*out = static_cast<state_enum_t>(state_idx);
	}
}

}

}

