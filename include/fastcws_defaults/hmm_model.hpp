// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "fastcws/hmm.hpp"
#include "fastcws/suspendable_region.hpp"
#include "fastcws_defaults/definitions.hpp"

namespace fastcws {

namespace defaults {

using hmm_model_region_t = suspendable_region::managed_region<seat_model>;
using hmm_model_t = hmm::wseg_4tag::model<
	fastcws::hmm::basic_normalizer<uint64_t, double>,
	suspendable_region::allocator<int, hmm_model_region_t>
>;

extern hmm_model_t* hmm_model;
void init_hmm_model() noexcept;

};

};






