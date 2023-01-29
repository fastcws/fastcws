// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include "fastcws/bindings/containers.hpp"

#ifndef FASTCWS_HAS_BOOST

#error "If you want to use fastcws::defaults, Boost library must be present"

#endif

#include "fastcws_defaults/hmm_model.hpp"
#include "fastcws_defaults/freq_dict.hpp"

namespace fastcws {

namespace defaults {

void init() noexcept {
	init_hmm_model();
	init_freq_dict();
}

};

};

