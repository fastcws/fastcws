// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <type_traits>

namespace fastcws {

namespace suspendable_region {

template <
	size_t Alignment = sizeof(uintmax_t),
	size_t SkipBytes = 0
>
struct basic_region {
	template <class T>
	using pointer = std::add_pointer_t<T>;
	using void_pointer = pointer<void>;

	std::vector<char> maybe_mem_;

	char *const mem_begin_;
	char *const mem_end_;
	
	std::atomic<char*> used_end_;

	basic_region(size_t bytes)
	: maybe_mem_(bytes, '\0'),
	  mem_begin_(reinterpret_cast<char *>(maybe_mem_.data())),
	  mem_end_(reinterpret_cast<char *>(maybe_mem_.data()) + bytes),
	  used_end_(reinterpret_cast<char *>(maybe_mem_.data()) + SkipBytes)
	{}

	// unowning region, created from preloaded mem view
	basic_region(char *view, size_t len, size_t used)
	: mem_begin_(view),
	  mem_end_(view + len),
	  used_end_(view + used)
	{}

	basic_region(std::vector<char> mem, size_t used)
	: maybe_mem_(std::move(mem)),
	  mem_begin_(reinterpret_cast<char *>(maybe_mem_.data())),
	  mem_end_(reinterpret_cast<char *>(maybe_mem_.data() + maybe_mem_.size())),
	  used_end_(reinterpret_cast<char *>(maybe_mem_.data() + used))
	{}

	basic_region(const basic_region&) = delete;
	basic_region(basic_region&&) = delete;
	basic_region& operator=(const basic_region&) = delete;
	basic_region& operator=(basic_region&&) = delete;

	uintptr_t base() const noexcept {
		return reinterpret_cast<uintptr_t>(mem_begin_);
	}

	const char* data() const noexcept {
		return mem_begin_;
	}

	size_t used() const noexcept {
		return used_end_.load() - mem_begin_;
	}

	void_pointer allocate(size_t bytes) noexcept {
		if (bytes == 0) {
			return nullptr;
		}

		char *used_end = used_end_.load();
__retry:
		char* aligned_end = used_end;
		if constexpr (Alignment > 1) {
			uintptr_t off = reinterpret_cast<uintptr_t>(used_end) % Alignment;
			if (off != 0) /* TODO ET_POOL_LIKELY */ {
				aligned_end += Alignment - off;
				if (aligned_end > mem_end_) {
					return nullptr;
				}
			}
		}

		char *new_end = aligned_end + bytes;
		if (new_end > mem_end_) {
			return nullptr;
		}
		if (used_end_.compare_exchange_weak(used_end, new_end)) {
			return reinterpret_cast<void *>(aligned_end);
		}
		goto __retry;
	}
};

}

}

