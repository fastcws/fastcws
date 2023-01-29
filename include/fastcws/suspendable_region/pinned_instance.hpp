// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <array>
#include <cassert>

namespace fastcws {

namespace suspendable_region {

// crtp helper
template <class T>
struct enable_pinned_instance {

	static T*& instance_ptr() noexcept {
		static T* instance = nullptr;
		return instance;
	}

	static void pin(T& obj) noexcept {
		assert(instance_ptr() == nullptr);
		instance_ptr() = &obj;
	}

	static void unpin() noexcept {
		instance_ptr() = nullptr;
	}

	static T& get_instance() noexcept {
		assert(instance_ptr() != nullptr);
		return *instance_ptr();
	}
};

}

}

