// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <limits>

namespace fastcws {

namespace rep_aware {

// representation-aware std::string_view, as mentioned in https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0773r0.html
template <
	class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT>
>
struct basic_string_view {
	using value_type = CharT;
	using reference = CharT&;
	using const_reference = const CharT&;

	using allocator_traits = std::allocator_traits<Allocator>;
	using pointer = typename allocator_traits::pointer;
	using const_pointer = typename allocator_traits::const_pointer;
	using size_type = typename allocator_traits::size_type;
	using difference_type = typename allocator_traits::difference_type;

	using const_iterator = const_pointer;
	using iterator = const_iterator;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using reverse_iterator = const_reverse_iterator;

	using traits_type = Traits;

	static constexpr size_type npos = std::numeric_limits<size_type>::max();
	static constexpr size_type max_size_ = npos - 1;
	const_pointer data_;
	size_type len_;

	constexpr basic_string_view() noexcept : data_(nullptr), len_(0) {}
	constexpr basic_string_view(const basic_string_view&) noexcept = default;
	basic_string_view& operator=(const basic_string_view&) noexcept = default;

	constexpr basic_string_view(const_pointer str)
		: data_(str), len_(traits_type::length(&*str)) {}
	constexpr basic_string_view(const_pointer str, size_type len)
		: data_(str), len_(len) {}
	basic_string_view(const std::string& str)
		: data_(str.data()), len_(str.size()) {}

	constexpr const_iterator begin() const noexcept {
		return data_;
	}
	constexpr const_iterator end() const noexcept {
		return data_ + len_;
	}
	constexpr const_iterator cbegin() const noexcept {
		return data_;
	}
	constexpr const_iterator cend() const noexcept {
		return data_ + len_;
	}

	const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator(end());
	}
	const_reverse_iterator crbegin() const noexcept {
		return const_reverse_iterator(end());
	}
	const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(begin());
	}
	const_reverse_iterator crend() const noexcept {
		return const_reverse_iterator(begin());
	}

	constexpr const_reference operator[](size_type pos) const {
		return data_[pos];
	}
	const_reference at(size_type pos) const {
		if (pos > len_) {
			throw std::out_of_range("fastcws::basic_string_view::at");
		}
		return data_[pos];
	}
	constexpr const_reference front() const {
		return data_[0];
	}
	constexpr const_reference back() const {
		return data_[len_ - 1];
	}
	constexpr const_pointer data() const noexcept {
		return data_;
	}

	constexpr size_type size() const noexcept {
		return len_;
	}
	constexpr size_type length() const noexcept {
		return len_;
	}
	constexpr size_type max_size() const noexcept {
		return max_size_;
	}
	constexpr bool empty() const noexcept {
		return len_ == 0;
	}

	void remove_prefix(size_type n) {
		data_ = data_ + n;
		len_ -= n;
	}
	void remove_suffix(size_type n) {
		len_ -= n;
	}

	void swap(basic_string_view& s) noexcept {
		std::swap(data_, s.data_);
		std::swap(len_, s.len_);
	}

	size_type copy(pointer dest, size_type count, size_type pos = 0) const {
		if (pos > len_) {
			throw std::out_of_range("fastcws::basic_string_view::copy");
		}
		const size_type rcount = std::min(count, len_ - pos);
		traits_type::copy(&*dest, &*(data_ + pos), rcount);
		return rcount;
	}

	basic_string_view substr(size_type pos = 0, size_type count = npos) const {
		if (pos > len_) {
			throw std::out_of_range("fastcws::basic_string_view::substr");
		}
		const size_type rcount = std::min(count, len_ - pos);
		if (rcount > 0) {
			return basic_string_view{data_ + difference_type(pos), rcount};
		}
		return basic_string_view{};
	}

	static int _str_compare(const_pointer ptr1, size_type len1, const_pointer ptr2, size_type len2) noexcept {
		const size_type cmp_len = std::min(len1, len2);
		const int cmp = traits_type::compare(&*ptr1, &*ptr2, cmp_len);
		return cmp == 0 ? static_cast<int>(len1 - len2) : cmp;
	}
	int compare(basic_string_view s) const noexcept {
		return _str_compare(data_, len_, s.data_, s.len_);
	}
	int compare(size_type pos1, size_type count1, basic_string_view s) const {
		return substr(pos1, count1).compare(s);
	}
	int compare(size_type pos1, size_type count1, basic_string_view s, size_type pos2, size_type count2) const {
		return substr(pos1, count1).compare(s.substr(pos2, count2));
	}
	int compare(const_pointer s) const {
		return compare(basic_string_view(s));
	}
	int compare(size_type pos1, size_type count1, const_pointer s) const {
		return substr(pos1, count1).compare(basic_string_view(s));
	}
	int compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const {
		return substr(pos1, count1).compare(basic_string_view(s, count2));
	}

	bool starts_with(basic_string_view s) const noexcept {
		return len_ >= s.len_ && substr(0, s.len_) == s;
	}
	bool starts_with(value_type c) const noexcept {
		return !empty() && traits_type::eq(front(), c);
	}
	bool starts_with(const_pointer s) const {
		return starts_with(basic_string_view{s});
	}

	bool ends_with(basic_string_view s) const noexcept {
		return len_ >= s.len_ && substr(len_ - s.len_, npos) == s;
	}
	bool ends_with(value_type c) const noexcept {
		return !empty() && traits_type::eq(back(), c);
	}
	bool ends_with(const_pointer s) const {
		return ends_with(basic_string_view{s});
	}

	size_type find(basic_string_view s, size_type pos = 0) const noexcept {
		if (pos > len_ || s.len_ > (len_ - pos)) {
			return npos;
		}
		if (s.empty()) {
			return pos;
		}
		for (;;) {
			if (traits_type::compare(&*(data_ + pos), &*s.data_, s.len_) == 0) {
				return pos;
			}
			if (pos == (len_ - s.len_)) {
				return npos;
			}
			pos++;
		}
	}
	size_type find(value_type c, size_type pos = 0) const noexcept {
		return find(basic_string_view(&c, 1), pos);
	}
	size_type find(const_pointer s, size_type pos, size_type n) const {
		return find(basic_string_view(s, n), pos);
	}
	size_type find(const_pointer s, size_type pos = 0) const {
		return find(basic_string_view(s), pos);
	}
	size_type rfind(basic_string_view s, size_type pos = npos) const noexcept {
		if (s.empty()) {
			return std::min(pos, len_);
		}
		if (s.len_ > std::min(pos, len_)) {
			return npos;
		}
		pos = std::min(pos, len_ - s.len_);
		for (;;) {
			if (traits_type::compare(&*(data_ + pos), &*s.data_, s.len_) == 0) {
				return pos;
			}
			if (pos == 0) {
				return npos;
			}
			pos--;
		}
	}
	size_type rfind(value_type c, size_type pos = npos) const noexcept {
		return rfind(basic_string_view(&c, 1), pos);
	}
	size_type rfind(const_pointer s, size_type pos, size_type n) const {
		return rfind(basic_string_view(s, n), pos);
	}
	size_type rfind(const_pointer s, size_type pos = npos) const {
		return rfind(basic_string_view(s), pos);
	}
	size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept {
		for (; pos < len_; pos++) {
			if (traits_type::find(&*s.data_, s.len_, data_[pos]) != nullptr) {
				return pos;
			}
		}
		return npos;
	}
	size_type find_first_of(value_type c, size_type pos = 0) const noexcept {
		return find_first_of(basic_string_view(&c, 1), pos);
	}
	size_type find_first_of(const_pointer s, size_type pos, size_type n) const {
		return find_first_of(basic_string_view(s, n), pos);
	}
	size_type find_first_of(const_pointer s, size_type pos = 0) const {
		return find_first_of(basic_string_view(s), pos);
	}
	size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept {
		if (empty()) {
			return npos;
		}
		pos = std::min(pos, len_ - 1);
		for (;;) {
			if (traits_type::find(&*s.data_, s.len_, data_[pos]) != nullptr) {
				return pos;
			}
			if (pos == 0) {
				return npos;
			}
			pos--;
		}
	}
	size_type find_last_of(value_type c, size_type pos = npos) const noexcept {
		return find_last_of(basic_string_view(&c, 1), pos);
	}
	size_type find_last_of(const_pointer s, size_type pos, size_type n) const {
		return find_last_of(basic_string_view(s, n), pos);
	}
	size_type find_last_of(const_pointer s, size_type pos = npos) const {
		return find_last_of(basic_string_view(s), pos);
	}
	size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept {
		for (; pos < len_; pos++) {
			if (traits_type::find(&*s.data_, s.len_, data_[pos]) == nullptr) {
				return pos;
			}
		}
		return npos;
	}
	size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept {
		return find_first_not_of(basic_string_view(&c, 1), pos);
	}
	size_type find_first_not_of(const_pointer s, size_type pos,
			size_type n) const {
		return find_first_not_of(basic_string_view(s, n), pos);
	}
	size_type find_first_not_of(const_pointer s, size_type pos = 0) const {
		return find_first_not_of(basic_string_view(s), pos);
	}
	size_type find_last_not_of(basic_string_view s, size_type pos = npos) const noexcept {
		if (empty()) {
			return npos;
		}
		pos = std::min(pos, len_ - 1);
		for (;;) {
			if (traits_type::find(&*s.data_, s.len_, data_[pos]) == nullptr) {
				return pos;
			}
			if (pos == 0) {
				return npos;
			}
			pos--;
		}
	}
	size_type find_last_not_of(value_type c,
			size_type pos = npos) const noexcept {
		return find_last_not_of(basic_string_view(&c, 1), pos);
	}
	size_type find_last_not_of(const_pointer s, size_type pos,
			size_type n) const {
		return find_last_not_of(basic_string_view(s, n), pos);
	}
	size_type find_last_not_of(const_pointer s, size_type pos = npos) const {
		return find_last_not_of(basic_string_view(s), pos);
	}

	friend inline bool operator==(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) == 0;
	}
	friend inline bool operator!=(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) != 0;
	}

	friend inline bool operator<(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) < 0;
	}
	friend inline bool operator>(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) > 0;
	}
	friend inline bool operator<=(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) <= 0;
	}
	friend inline bool operator>=(basic_string_view a, basic_string_view b) noexcept {
		return a.compare(b) >= 0;
	}

	friend std::ostream& operator<<(std::ostream& os, basic_string_view s) {
		std::ostream::sentry sentry{os};
		if (!sentry) {
			return os;
		}
		size_t padding = 0;
		char filler = os.fill();
		if (static_cast<std::streamsize>(s.size()) < os.width()) {
			padding = os.width() - s.size();
		}
		bool align_left = os.flags() & std::ios_base::left;
		if (padding > 0 && !align_left) {
			while (padding--) {
				os.put(filler);
			}
		}
		os.write(s.data(), s.size());
		if (padding > 0 && align_left) {
			while (padding--) {
				os.put(filler);
			}
		}
		os.width(0);
		return os;
	}
};

using string_view = basic_string_view<char>;

}

}

namespace std {

template <class Allocator>
struct hash<fastcws::rep_aware::basic_string_view<char, std::char_traits<char>, Allocator>> : std::hash<std::string_view> {
	using base_t = std::hash<std::string_view>;
	using sv_t = fastcws::rep_aware::basic_string_view<char, std::char_traits<char>, Allocator>;

	size_t operator()(sv_t s) const {
		return base_t::operator()(std::string_view{s.data(), s.length()});
	}
};

}

