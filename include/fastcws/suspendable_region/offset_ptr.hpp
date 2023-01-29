// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <atomic>
#include <memory>

namespace fastcws {

namespace suspendable_region {

template <class T, class GetBaseProxy>
struct basic_offset_ptr {
	using element_type = T;
	using value_type = std::remove_cv<T>;
	using pointer = T*;
	using reference = std::add_lvalue_reference_t<T>;
	using iterator_category = std::random_access_iterator_tag;
	using difference_type = ptrdiff_t;

private:
	// here we use 0 to resemble null, by skipping a few leading bytes in region
	ptrdiff_t offset_ = 0;

	struct construct_from_offset{};
	constexpr basic_offset_ptr(ptrdiff_t off, construct_from_offset)
	: offset_(off)
	{}

public:
	constexpr basic_offset_ptr() noexcept = default;
	constexpr basic_offset_ptr(std::nullptr_t) noexcept {}
	basic_offset_ptr(const basic_offset_ptr&) noexcept = default;
	basic_offset_ptr& operator=(const basic_offset_ptr&) noexcept = default;

	static basic_offset_ptr from_offset(ptrdiff_t off) noexcept {
		return {off, construct_from_offset{}};
	}

	static constexpr uintptr_t base() noexcept {
		return GetBaseProxy::get_base();
	}

	ptrdiff_t offset() const noexcept {
		return static_cast<ptrdiff_t>(offset_);
	}

	template <class U>
	static ptrdiff_t ptr_to_offset(U* ptr) noexcept {
		if (ptr == nullptr) {
			return 0;
		}
		return reinterpret_cast<uintptr_t>(ptr) - base();
	}

	// begin type casting
	// some of these are not required by standard, yet libstdc++ and libc++ do not work without them
	template <class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	basic_offset_ptr(const basic_offset_ptr<U, GetBaseProxy>& o) noexcept
	: basic_offset_ptr(o.offset(), construct_from_offset{}) {}

	template <class U = T, class = std::enable_if_t<!std::is_void_v<U>>>
	basic_offset_ptr(const basic_offset_ptr<void, GetBaseProxy>& o) noexcept
	: basic_offset_ptr(o.offset(), construct_from_offset{}) {}

	template <class U, class = std::enable_if_t<std::is_convertible_v<U*, T*>>>
	basic_offset_ptr& operator=(basic_offset_ptr<U, GetBaseProxy> o) noexcept {
		offset_ = o.offset();
		return *this;
	};

	template <class U>
	basic_offset_ptr(U* o) noexcept
	: basic_offset_ptr(ptr_to_offset(o), construct_from_offset{}) {}

	template <class U = T, class enabler = std::enable_if_t<!std::is_void_v<U>>>
	static basic_offset_ptr pointer_to(U& obj) noexcept {
		return from_offset(ptr_to_offset(&obj));
	}

	operator pointer() const noexcept {
		return get();
	}
	// end type casting

	// begin fancy pointer overloads
	pointer get() const noexcept {
		return reinterpret_cast<pointer>(base() + offset_);
	}
	reference operator*() const noexcept { return *get(); }
	pointer operator->() const noexcept { return get(); }
	// end fancy pointer overloads

	// begin NullablePointer
	basic_offset_ptr& operator=(std::nullptr_t) noexcept {
		offset_ = 0;
		return *this;
	};

	bool operator==(std::nullptr_t) const noexcept {
		return offset_ == 0;
	}

	bool operator!=(std::nullptr_t) const noexcept {
		return offset_ != 0;
	}

	explicit operator bool() const { return offset_ != 0; }
	// end NullablePointer

	// begin EqualComparable
	template <class U>
	bool operator==(basic_offset_ptr<U, GetBaseProxy> o) const noexcept {
		return offset_ == o.offset();
	}

	template <class U>
	bool operator!=(basic_offset_ptr<U, GetBaseProxy> o) const noexcept {
		return offset_ != o.offset();
	}
	// end EqualComparable

	// begin arithmetics RandomAccessIterator & ContiguousIterator
	basic_offset_ptr& operator+=(difference_type diff) noexcept {
		offset_ += diff * sizeof(T);
		return *this;
	}

	basic_offset_ptr& operator-=(difference_type diff) noexcept  {
		offset_ -= diff * sizeof(T);
		return *this;
	}

	basic_offset_ptr& operator++(void) noexcept  {
		offset_ += sizeof(T);
		return *this;
	}

	basic_offset_ptr& operator--(void) noexcept  {
		offset_ -= sizeof(T);
		return *this;
	}

	basic_offset_ptr operator++(int) noexcept  {
		offset_ += sizeof(T);
		return from_offset(offset_ - sizeof(T));
	}

	basic_offset_ptr operator--(int) noexcept {
		offset_ -= sizeof(T);
		return from_offset(offset_ + sizeof(T));
	}

	friend basic_offset_ptr operator+(difference_type diff, basic_offset_ptr ptr) noexcept {
		return from_offset(ptr.offset_ + diff * sizeof(T));
	}

	friend basic_offset_ptr operator+(basic_offset_ptr ptr, difference_type diff) noexcept {
		return from_offset(ptr.offset_ + diff * sizeof(T));
	}

	reference operator[](difference_type diff) const noexcept {
		return *from_offset(offset_ + diff * sizeof(T));
	}

	friend basic_offset_ptr operator-(basic_offset_ptr ptr, difference_type diff) noexcept {
		return from_offset(ptr.offset_ - diff * sizeof(T));
	}

	friend difference_type operator-(basic_offset_ptr lhs, basic_offset_ptr rhs) noexcept {
		return (lhs.offset_ - rhs.offset_) / sizeof(T);
	}
	// end arithmetics RandomAccessIterator & ContiguousIterator

};

// crtp helper
template <class GetBaseProxy>
struct enable_offset_ptr {
	template <class T>
	using offset_ptr = basic_offset_ptr<T, GetBaseProxy>;
};

}

}

