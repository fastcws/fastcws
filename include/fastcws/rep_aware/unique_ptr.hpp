#pragma once

#include <memory>
#include <utility>
#include <type_traits>

namespace fastcws {

namespace rep_aware {

template <class T, class Allocator>
struct deletor : Allocator {
	using allocator_t = Allocator;
	using allocator_traits = std::allocator_traits<allocator_t>;
	using pointer = typename allocator_traits::pointer;

	void operator()(pointer ptr) noexcept {
		if (ptr == nullptr) {
			return;
		}
		allocator_traits::destroy(*this, &*ptr);
		allocator_traits::deallocate(*this, &*ptr, 1);
	}
};

template <class T, class Allocator>
struct unbounded_array_deletor : Allocator {
	using allocator_t = Allocator;
	using allocator_traits = std::allocator_traits<allocator_t>;
	using pointer = typename allocator_traits::pointer;
	using size_type = typename allocator_traits::size_type;

	size_type n_;

	unbounded_array_deletor(size_t n)
		: n_(n)
	{}

	void operator()(pointer ptr) noexcept {
		if (ptr == nullptr) {
			return;
		}
		for (size_t i = 0; i < n_; i++) {
			allocator_traits::destroy(*this, &ptr[i]);
		}
		allocator_traits::deallocate(*this, &*ptr, n_);
	}
};

namespace detail {

template<class>
constexpr bool is_unbounded_array_v = false;

template<class T>
constexpr bool is_unbounded_array_v<T[]> = true;
 
template<class>
constexpr bool is_bounded_array_v = false;

template<class T, std::size_t N>
constexpr bool is_bounded_array_v<T[N]> = true;

}

template <class T, class Allocator, class = void>
struct unique_ptr_of{};

template <class T, class Allocator>
struct unique_ptr_of<T, Allocator, std::enable_if_t<!std::is_array_v<T>>>{
	using type = std::unique_ptr<T, deletor<T, Allocator>>;
};

template <class T, class Allocator>
struct unique_ptr_of<T, Allocator, std::enable_if_t<detail::is_unbounded_array_v<T>>>{
	using type = std::unique_ptr<T, unbounded_array_deletor<T, Allocator>>;
};

template <class T, class Allocator>
using unique_ptr = typename unique_ptr_of<T, Allocator>::type;
 
template<class T, class Allocator, class... Args>
std::enable_if_t<!std::is_array_v<T>, unique_ptr<T, Allocator>> make_unique(Allocator allocator, Args&&... args)
{
	using allocator_traits = std::allocator_traits<Allocator>;
	using pointer = typename allocator_traits::pointer;

	pointer ptr = allocator_traits::allocate(allocator, 1);
	allocator_traits::construct(allocator, &*ptr, std::forward<Args>(args)...);
	return unique_ptr<T, Allocator>(ptr);

}

template<class T, class Allocator, class... Args>
std::enable_if_t<detail::is_unbounded_array_v<T>, unique_ptr<T, Allocator>> make_unique(Allocator allocator,
		typename std::allocator_traits<Allocator>::size_type n)
{
	using array_element_t = std::remove_extent_t<T>;
	using allocator_traits = std::allocator_traits<Allocator>;
	using pointer = typename allocator_traits::pointer;

	pointer ptr = allocator_traits::allocate(allocator, n);
	for (size_t i = 0; i < n; i++) {
		allocator_traits::construct(allocator, &ptr[i]);
	}
    return unique_ptr<T, Allocator>(ptr, unbounded_array_deletor<T, Allocator>{n});
}
 
template<class T, class Allocator, class... Args>
std::enable_if_t<detail::is_bounded_array_v<T>> make_unique(Args&&...) = delete;

}

}

