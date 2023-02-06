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

	// marks the minium valid errc
	last_errc = -4
};

namespace details {

inline constexpr const char *const error_messages[] = {
	//  0
	"no error",
	// -1 = errc::internal_error
	"irrecoverable, unexpected internal error",
	// -2 = errc::io_error
	"file i/o failed",
	// -3 = errc::bad_encoding
	"input is not a valid utf-8 sequence",
	// -4 = errc::overflow_error
	"attempting to access an object beyong its limit"
};

}

struct error_category : std::error_category
{
	const char* name() const noexcept override {
		return "fastcws";
	}

	std::string message(int ev) const override {
		if ((ev < static_cast<int>(errc::last_errc)) || (ev > 0)) {
			return "undefined error code";
		}
		return std::string{details::error_messages[-ev]};
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

