// SPDX-License-Identifier: BSD-2-Clause

#pragma once

#include <new>
#include <utility>
#include <type_traits>

namespace fastcws {

namespace suspendable_region {

template <class T, class Region>
struct allocator {
	using value_type = T;
	using pointer = typename Region::template pointer<T>;
	using const_pointer = typename Region::template pointer<const T>;
	using void_pointer = typename Region::template pointer<void>;
	using const_void_pointer = typename Region::template pointer<const void>;

	using size_type = size_t;
	using difference_type = ptrdiff_t;

	allocator() {}
	allocator(const allocator &) = default;
	template <typename U>
	allocator(const allocator<U, Region> &o)
	{}

	template <class U> struct rebind{
		using other = allocator<U, Region>;
	};

	pointer allocate(size_type n, const_void_pointer cvp = nullptr) const {
		void_pointer ptr = Region::get_instance().allocate(n * sizeof(T));
		if (ptr == nullptr) {
			throw std::bad_alloc{};
		}
		return static_cast<pointer>(ptr);
	}

	void deallocate(pointer p, size_type n) const noexcept {
		if (p == nullptr) {
			return;
		}
		Region::get_instance().free(static_cast<void_pointer>(p));
	}

	template <class OtherT, class OtherRegion>
	bool operator==(allocator<OtherT, OtherRegion> o) const noexcept {
		if constexpr (std::is_same_v<T, OtherT> && std::is_same_v<Region, OtherRegion>) {
			// The region is pinned on seats, so same type means same object
			return true;
		}
		return false;
	}

	template <class OtherT, class OtherRegion>
	bool operator!=(allocator<OtherT, OtherRegion> o) const noexcept {
		return !operator==(o);
	}
};

template <class Region>
struct allocator_wrapper {
	allocator_wrapper() {}

	template <class T>
	allocator<T, Region> get() const noexcept {
		return allocator<T, Region>{};
	}
};

template <class Region>
allocator_wrapper<Region> allocator_of(Region& region) {
	return {};
}

}

}

