#pragma once

#include <system_error>

#include "fastcws/error/error_code.hpp"

namespace fastcws {
	namespace exception {
		struct bad_encoding : std::system_error {
			bad_encoding() : std::system_error(std::error_code{fastcws::errc::bad_encoding}) {}
		};
	}
}

