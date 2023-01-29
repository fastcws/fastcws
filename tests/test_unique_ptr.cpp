#include "gtest/gtest.h"

#include "fastcws/rep_aware/unique_ptr.hpp"

static int dtor_called = 0;

struct A {
	A(){}
	~A() { dtor_called++; }
	A(const A&) = delete;
	A(A&&) = delete;
};

TEST(unique_ptr, general) {
	dtor_called = 0;
	{
		using namespace fastcws;
		auto ptr0 = rep_aware::make_unique<A>(std::allocator<A>{});
		rep_aware::unique_ptr<A, std::allocator<A>> ptr1(std::move(ptr0));
	}
	EXPECT_EQ(dtor_called, 1);
}

TEST(unique_ptr, unbounded_array) {
	dtor_called = 0;
	{
		using namespace fastcws;
		auto ptr0 = rep_aware::make_unique<A[]>(std::allocator<A>{}, 10);
		rep_aware::unique_ptr<A[], std::allocator<A>> ptr1(std::move(ptr0));
	}
	EXPECT_EQ(dtor_called, 10);
}

