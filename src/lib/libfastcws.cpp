// SPDX-License-Identifier: BSD-2-Clause

#include <iostream>
#include <fstream>
#include <string_view>
#include <optional>

#include "fastcws.hpp"
#include "fastcws_defaults.hpp"

extern "C" {

#define FASTCWS_EXPORTING
#include "./libfastcws.h"

typedef struct fastcws_ctx_s {
	std::optional<fastcws::freq_dict::dict<>> dict_ptr;
	std::optional<fastcws::hmm::wseg_4tag::model<>> model_ptr;
} fastcws_ctx;

typedef struct fastcws_result_s {
	std::vector<std::string_view> words;
	size_t cursor = 0;
} fastcws_result;

void fastcws_init() {
	fastcws::defaults::init();
}

fastcws_ctx *fastcws_alloc_ctx() {
	return new fastcws_ctx();
}

void fastcws_ctx_free(fastcws_ctx* ctx) {
	delete ctx;
}

int fastcws_load_freq_dict(const char *filename, fastcws_ctx* ctx) {
	std::ifstream f{filename};
	if (!f.good()) {
		return FASTCWS_E_IO;
	}
	try {
		ctx->dict_ptr.emplace(fastcws::freq_dict::load_dict(f));
	} catch (...) {
		return FASTCWS_E_IO;
	}
	return FASTCWS_OK;
}

int fastcws_load_hmm_model(const char *filename, fastcws_ctx* ctx) {
	std::ifstream f{filename};
	if (!f.good()) {
		return FASTCWS_E_IO;
	}
	try {
		ctx->model_ptr.emplace(fastcws::hmm::wseg_4tag::load(f));
	} catch (...) {
		return FASTCWS_E_IO;
	}
	return FASTCWS_OK;
}

fastcws_result *fastcws_alloc_result() {
	return new fastcws_result();
}

int fastcws_word_break(const char *cstr, fastcws_result* result) {
	try {
		result->words.resize(0);
		result->cursor = 0;
		fastcws::word_break(std::string_view{cstr}, std::back_inserter(result->words), *fastcws::defaults::freq_dict, *fastcws::defaults::hmm_model);
	} catch (std::system_error e) {
		if (e.code().category() != fastcws::category) {
			return FASTCWS_E_INTERNAL;
		}
		return e.code().value();
	}
	return FASTCWS_OK;
}

int fastcws_word_break2(const char *cstr, fastcws_result *result, const fastcws_ctx* ctx) {
	try {
		if (ctx->dict_ptr) {
			if (ctx->model_ptr) {
				fastcws::word_break(std::string_view{cstr}, std::back_inserter(result->words),
						*ctx->dict_ptr, *ctx->model_ptr);
			} else {
				fastcws::word_break(std::string_view{cstr}, std::back_inserter(result->words),
						*ctx->dict_ptr, *fastcws::defaults::hmm_model);
			}
		} else {
			if (ctx->model_ptr) {
				fastcws::word_break(std::string_view{cstr}, std::back_inserter(result->words),
						*fastcws::defaults::freq_dict, *ctx->model_ptr);
			} else {
				fastcws::word_break(std::string_view{cstr}, std::back_inserter(result->words),
						*fastcws::defaults::freq_dict, *fastcws::defaults::hmm_model);
			}
		}
	} catch (std::system_error e) {
		if (e.code().category() != fastcws::category) {
			return FASTCWS_E_INTERNAL;
		}
		return e.code().value();
	}
	return FASTCWS_OK;
}

int fastcws_result_next(fastcws_result* result, const char** word_begin, size_t* word_len) {
	if (result->cursor >= result->words.size()) {
		*word_begin = nullptr;
		*word_len = 0;
		return FASTCWS_E_OVERFLOW;
	}
	const auto& sv = result->words[result->cursor];
	*word_begin = sv.data();
	*word_len = sv.size();
	result->cursor++;
	return FASTCWS_OK;
}

void fastcws_result_free(fastcws_result* result) {
	delete result;
}

const char* fastcws_strerr(int err) {
	if ((err > 0) || (err < static_cast<int>(fastcws::errc::last_errc))) {
		return "invalid error code";
	}
	return fastcws::details::error_messages[-err];
}

}

