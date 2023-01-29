// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <cmath>

namespace fastcws {

template <class Weight> struct calc_log2;

template <>
struct calc_log2<float> {
	static float log2(uint64_t f) {
		return std::log2f(static_cast<float>(f));
	}
};

template <>
struct calc_log2<double> {
	static double log2(uint64_t f) {
		return std::log2(static_cast<double>(f));
	}
};

template <>
struct calc_log2<long double> {
	static long double log2(uint64_t f) {
		return std::log2(f);
	}
};

}


