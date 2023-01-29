// SPDX-License-Identifier: BSD-2-Clause

#pragma once

extern "C" {

#include "zlib.h"

}

#include <cassert>

namespace fastcws {

namespace defaults {

struct zlib_compressor {
	static size_t compress(const char* input, size_t input_size, char* output, size_t output_size) noexcept {
		::uLongf output_size_c = static_cast<::uLongf>(output_size);
		int ret = ::compress(reinterpret_cast<::Bytef*>(output), &output_size_c,
				reinterpret_cast<const ::Bytef*>(input), static_cast<::uLong>(input_size));
		assert(ret == Z_OK);
		return static_cast<size_t>(output_size_c);
	}

	static size_t decompress(const char* input, size_t input_size, char* output, size_t output_size) noexcept {
		::uLongf output_size_c = static_cast<::uLongf>(output_size);
		int ret = ::uncompress(reinterpret_cast<::Bytef*>(output), &output_size_c,
				reinterpret_cast<const ::Bytef*>(input), static_cast<::uLong>(input_size));
		assert(ret == Z_OK);
		return static_cast<size_t>(output_size_c);
	}
};

using compressor = zlib_compressor;

};

};

