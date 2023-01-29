// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <numeric>

#include "fastcws/misc/log2.hpp"

namespace fastcws {

namespace hmm {

template <class InputType, class OutputType>
struct basic_normalizer {
	using input_t = InputType;
	using output_t = OutputType;

	template <class InputTypeIterable, class OutputTypeIterable> /* TODO concepts */
	static void normalize(const InputTypeIterable& in_begin, const InputTypeIterable& in_end, OutputTypeIterable out_it) noexcept {
		input_t sum = std::accumulate(in_begin, in_end, input_t(0));
		output_t log_sum = calc_log2<output_t>::log2(sum);
		auto in_it = in_begin;
		while (in_it != in_end) {
			if (sum == 0) {
				*out_it = 0;
			} else {
				*out_it = calc_log2<output_t>::log2(static_cast<uint64_t>(*in_it)) - log_sum;
			}
			in_it++;
			out_it++;
		}
	}
};

}

}

