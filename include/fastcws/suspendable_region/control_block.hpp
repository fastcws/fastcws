// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <array>
#include <utility>
#include <algorithm>

namespace fastcws {

namespace suspendable_region {

template <
	class Key = uint16_t,
	class Value = ptrdiff_t,
	size_t MaxSize = 32
>
struct control_block {
	std::array<std::pair<Key, Value>, MaxSize> arr_{0};
	size_t sz = 0;

	void put(Key k, Value v) noexcept {
		arr_[sz++] = std::pair<Key, Value>{k, v};
	}

	Value get(Key k) noexcept {
		auto arr_end = arr_.begin() + sz;
		std::sort(arr_.begin(), arr_end);
		auto it = std::lower_bound(arr_.begin(), arr_end, k, [](const std::pair<Key, Value> &ele, const Key& k) -> bool { return ele.first < k; });
		if (it == arr_end) {
			return 0;
		}
		return it->second;
	}
};

}

}

