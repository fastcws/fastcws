// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <string_view>

#include "fastcws/freq_dict.hpp"
#include "fastcws/word_dag.hpp"
#include "fastcws/hmm.hpp"
#include "fastcws/misc/rune_hopper.hpp"
#include "fastcws/misc/special_hopper.hpp"

namespace fastcws {

class no_dict_t{};
const inline no_dict_t no_dict{};
class no_hmm_model_t{};
const inline no_hmm_model_t no_hmm_model{};

template <
	class Dict,
	class HMMModel,
	class RuneHopper = rune_hopper::utf8_hopper,
	class WordDag = word_dag::dag<>
	>
WordDag build_dag(std::string_view sentence, const Dict& dict, const HMMModel& hmm_model) {
	using dag_t = WordDag;
	using weight_t = typename dag_t::weight_t;
	using rune_hopper_t = RuneHopper;
	using dict_t = Dict;
	using hmm_model_t = HMMModel;


	weight_t single_rune_edge_weight = 32;
	weight_t hmm_edge_weight = 16;
	if constexpr (!std::is_same_v<dict_t, no_dict_t>) {
		single_rune_edge_weight = dict.template suggest_single_rune_weight<dag_t>();
		hmm_edge_weight = dict.template suggest_hmm_model_weight<dag_t>();
	}
	
	WordDag dag{sentence};
	populate_rune_chain<RuneHopper>(dag, single_rune_edge_weight);
	add_special_edges(dag);
	if constexpr (!std::is_same_v<dict_t, no_dict_t>) {
		dict.add_edges(dag);
	}
	if constexpr (!std::is_same_v<hmm_model_t, no_hmm_model_t>) {
		hmm_model.template add_edges<dag_t, rune_hopper_t>(dag, hmm_edge_weight);
	}
	return dag;
}

template <class WordDag, class StringViewOutputIterator>
void word_break_by_dag(const WordDag& dag, StringViewOutputIterator out) {
	auto sp = kahn(dag);
	size_t ws = 0;
	for (auto we : sp.path) {
		*out = dag.sentence().substr(ws, we - ws);
		ws = we;
		out++;
	}
	*out = dag.sentence().substr(ws);
}

template <
	class Dict,
	class HMMModel,
	class StringViewOutputIterator,
	class RuneHopper = rune_hopper::utf8_hopper,
	class WordDag = word_dag::dag<>
>
void word_break(std::string_view sentence, StringViewOutputIterator out, const Dict& dict, const HMMModel& hmm_model) {
	auto dag = build_dag<Dict, HMMModel, RuneHopper, WordDag>(sentence, dict, hmm_model);
	word_break_by_dag(dag, out);
}

}

