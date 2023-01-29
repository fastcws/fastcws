// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <utility>
#include <type_traits>
#include <vector>
#include <mutex>
#include <memory>

#include "fastcws/suspendable_region/basic_region.hpp"
#include "fastcws/suspendable_region/offset_ptr.hpp"
#include "fastcws/suspendable_region/pinned_instance.hpp"
#include "fastcws/suspendable_region/control_block.hpp"

namespace fastcws {

namespace suspendable_region {

template <
	class Seat,
	size_t Alignment = sizeof(uintmax_t),
	size_t SkipBytes = sizeof(uintmax_t),
	class PointerTag = uint16_t
>
struct basic_managed_region
	: enable_offset_ptr<
		basic_managed_region<
			Seat,
			Alignment,
			SkipBytes,
			PointerTag
		>
	>, enable_pinned_instance<
		basic_managed_region<
			Seat,
			Alignment,
			SkipBytes,
			PointerTag
		>
	> {
	static_assert(SkipBytes > 0, "SkipBytes have to be positive to use offset_ptr");

	static constexpr size_t hdr_size_ = ((sizeof(size_t) + Alignment - 1) / Alignment) * Alignment;

	using pointer_tag_t = PointerTag;
	using control_block_t = control_block<pointer_tag_t, ptrdiff_t>;
	using basic_region_t = basic_region<Alignment, SkipBytes>;
	template <class T>
	using offset_ptr_t = typename enable_offset_ptr<basic_managed_region>::template offset_ptr<T>;
	using pinned_instance_t = enable_pinned_instance<basic_managed_region>;

	template <class T>
	using pointer = offset_ptr_t<T>;
	using void_pointer = pointer<void>;

	basic_region_t basic_region_;
	std::vector<std::pair<size_t, size_t>> freed_;
	std::mutex lock_freed_;

	basic_managed_region(size_t bytes)
		: basic_region_(bytes)
	{
		pinned_instance_t::pin(*this);

		control_block_t *cblock_ptr = reinterpret_cast<control_block_t*>(basic_region_.allocate(sizeof(control_block_t)));
		if (cblock_ptr == nullptr) {
			throw std::bad_alloc{};
		}
		assert(cblock_ptr == &cblock());
	}

	basic_managed_region(char *view, size_t len, size_t used, std::vector<std::pair<size_t, size_t>> freed)
		: basic_region_(view, len, used), freed_(freed)
	{
		pinned_instance_t::pin(*this);
	}

	basic_managed_region(std::vector<char> mem, size_t used, std::vector<std::pair<size_t, size_t>> freed)
		: basic_region_(std::move(mem), used), freed_(freed)
	{
		pinned_instance_t::pin(*this);
	}

	~basic_managed_region() {
		pinned_instance_t::unpin();
	}

	basic_managed_region(const basic_managed_region&) = delete;
	basic_managed_region(basic_managed_region&&) = delete;
	basic_managed_region& operator=(const basic_managed_region&) = delete;
	basic_managed_region& operator=(basic_managed_region&&) = delete;

	control_block_t& cblock() const noexcept {
		return *reinterpret_cast<control_block_t *>(basic_region_.base() + SkipBytes);
	}

	static basic_managed_region& get_instance() noexcept {
		return pinned_instance_t::get_instance();
	}

	static uintptr_t get_base() noexcept {
		return get_instance().base();
	}

	uintptr_t base() const noexcept {
		return basic_region_.base();
	}

	const char* data() const noexcept {
		return basic_region_.data();
	}

	size_t used() const noexcept {
		return basic_region_.used();
	}

	void_pointer allocate(size_t bytes) noexcept {
		char* ptr = reinterpret_cast<char*>(basic_region_.allocate(bytes + hdr_size_));
		if (ptr == nullptr) {
			return {};
		}
		size_t *size_hdr = reinterpret_cast<size_t *>(ptr);
		*size_hdr = bytes + hdr_size_;
		return void_pointer(reinterpret_cast<void*>(ptr + hdr_size_));
	}

	void free(void_pointer ptr) noexcept {
		if (ptr == nullptr) {
			return;
		}
		std::lock_guard lg_{lock_freed_};
		size_t size = *reinterpret_cast<size_t*>(reinterpret_cast<char*>(ptr.get()) - hdr_size_);
		freed_.emplace_back(ptr.offset() - hdr_size_, size);
	}

	template <class U>
	void tag_ptr(PointerTag tag, offset_ptr_t<U> ptr) noexcept {
		cblock().put(tag, ptr.offset());
	}

	template <class U>
	offset_ptr_t<U> retrieve_ptr(PointerTag tag) const noexcept {
		return offset_ptr_t<U>::from_offset(cblock().get(tag));
	}

	void _optimize_freed() noexcept {
		std::lock_guard lg_{lock_freed_};
		std::sort(freed_.begin(), freed_.end());
		size_t curr_off = 0;
		size_t curr_len = 0;
		decltype(freed_) optimized;
		for (auto [off, len] : freed_) {
			if (off == (curr_off + curr_len)) {
				curr_len += len;
			} else {
				if (curr_len != 0) {
					optimized.emplace_back(curr_off, curr_len);
				}
				curr_off = off;
				curr_len = len;
			}
		}
		if (curr_len != 0) {
			optimized.emplace_back(curr_off, curr_len);
		}
		freed_.swap(optimized);
	}

	void suspend(std::ostream& os) {
		_optimize_freed();
		size_t use = used();
		os.write(reinterpret_cast<char *>(&use), sizeof(use));
		size_t freed_list_size = freed_.size();
		os.write(reinterpret_cast<char *>(&freed_list_size), sizeof(freed_list_size));
		for (auto [off, len]: freed_) {
			os.write(reinterpret_cast<char *>(&off), sizeof(off));
			os.write(reinterpret_cast<char *>(&len), sizeof(len));
		}
		size_t written = 0;
		for (auto [off, len]: freed_) {
			if (off > written) {
				os.write(data() + written, off - written);
			}
			written = off + len;
		}
		if (use > written) {
			os.write(data() + written, use - written);
		}
	}

	static void _recover_after_read_use(std::istream& is, char* buffer, size_t buffer_size, const size_t use, std::vector<std::pair<size_t, size_t>>& freed) {
		if (use > buffer_size) {
			throw std::overflow_error{"buffer_size not capable of containing the whole image"};
		}
		size_t freed_list_size;
		is.read(reinterpret_cast<char*>(&freed_list_size), sizeof(freed_list_size));
		for (size_t i = 0; i < freed_list_size; i++) {
			size_t off, len;
			is.read(reinterpret_cast<char*>(&off), sizeof(off));
			is.read(reinterpret_cast<char*>(&len), sizeof(len));
			freed.emplace_back(off, len);
		}
		size_t next_read_to = 0;
		for (auto [off, len]: freed) {
			if (off > next_read_to) {
				is.read(buffer + next_read_to, off - next_read_to);
			}
			next_read_to = off + len;
		}
		if (use > next_read_to) {
			is.read(buffer + next_read_to, use - next_read_to);
		}
	}

};

template <
	class Seat,
	size_t Alignment = sizeof(uintmax_t),
	size_t SkipBytes = sizeof(uintmax_t),
	class PointerTag = uint16_t
>
struct managed_region {
	using basic_t = basic_managed_region<Seat, Alignment, SkipBytes, PointerTag>;

	std::unique_ptr<basic_t> pimpl_;

	using pointer_tag_t = typename basic_t::pointer_tag_t;
	using control_block_t = typename basic_t::control_block_t;
	template <class T>
	using pointer = typename basic_t::template pointer<T>;
	using void_pointer = typename basic_t::void_pointer;

	managed_region() {}
	managed_region(size_t bytes)
		: pimpl_(std::make_unique<basic_t>(bytes)) {}
	managed_region(std::unique_ptr<basic_t> ptr)
		: pimpl_{std::move(ptr)} {}

	control_block_t& cblock() const noexcept { return pimpl_->cblock(); }
	static basic_t &get_instance() { return basic_t::get_instance(); }
	static uintptr_t get_base() noexcept { return basic_t::get_base(); }
	uintptr_t base() const noexcept { return pimpl_->base(); }
	const char* data() const noexcept { return pimpl_->data(); }
	size_t used() const noexcept { return pimpl_->used(); }

	void_pointer allocate(size_t bytes) noexcept { return pimpl_->allocate(bytes); };
	void free(void_pointer ptr) noexcept { pimpl_->free(ptr); }

	template <class U>
	void tag_ptr(PointerTag tag, pointer<U> ptr) noexcept { pimpl_->tag_ptr(tag, ptr); }
	template <class U>
	pointer<U> retrieve_ptr(PointerTag tag) const noexcept { return pimpl_->template retrieve_ptr<U>(tag); }

	void suspend(std::ostream& os) { pimpl_->suspend(os); }
	static managed_region recover(std::istream& is, char* buffer, size_t buffer_size) {
		size_t use;
		is.read(reinterpret_cast<char*>(&use), sizeof(use));
		std::vector<std::pair<size_t, size_t>> freed;
		basic_t::_recover_after_read_use(is, buffer, buffer_size, use, freed);
		return managed_region(std::make_unique<basic_t>(buffer, buffer_size, use, freed));
	}

	static managed_region recover(std::istream& is) {
		size_t use;
		is.read(reinterpret_cast<char*>(&use), sizeof(use));
		std::vector<char> buffer;
		buffer.resize(use, 0);
		std::vector<std::pair<size_t, size_t>> freed;
		basic_t::_recover_after_read_use(is, buffer.data(), buffer.size(), use, freed);
		return managed_region(std::make_unique<basic_t>(buffer, use, freed));
	}
};

}

}

