// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <memory>

#include "fastcws/freq_dict.hpp"
#include "fastcws/suspendable_region.hpp"
#include "fastcws_defaults/definitions.hpp"

namespace fastcws {

namespace defaults {

using freq_dict_region_t = suspendable_region::managed_region<seat_dict>;
using freq_dict_t = freq_dict::dict<
	suspendable_region::allocator<int, freq_dict_region_t>,
	std::allocator<int>,
	true
>;

extern freq_dict_t* freq_dict;
void init_freq_dict() noexcept;

};

};






