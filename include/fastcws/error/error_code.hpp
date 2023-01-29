#pragma once

#include <type_traits>
#include <string>
#include <system_error>

namespace fastcws {

enum class errc : int {
	internal_error= -1,
	io_error = -2,
	bad_encoding = -3,
	overflow_error = -4,
};

struct error_category : std::error_category
{
	const char* name() const noexcept override {
		return "fastcws";
	}

	std::string message(int ev) const override {
		switch (static_cast<errc>(ev)) {
			case errc::internal_error:
				return "irrecoverable, unexpected internal error";
			case errc::io_error:
				return "file i/o failed";
			case errc::bad_encoding:
				return "input is not a valid utf-8 sequence";
			case errc::overflow_error:
				return "attempting to access an object beyong its limit";
			default:
				return "undefined error code";
		}
	}
};

const inline error_category category{};

inline std::error_code make_error_code(errc ev) {
	return {static_cast<int>(ev), category};
}

}

namespace std {
	template <>
	struct is_error_code_enum<fastcws::errc> : std::true_type {};
}

