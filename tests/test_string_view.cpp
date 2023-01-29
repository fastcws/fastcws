#include "gtest/gtest.h"

#include <exception>
#include <sstream>
#include <string>
#include <string_view>

#include "fastcws/rep_aware/string_view.hpp"

// #define TEST_STD_STRING_VIEW
#ifdef TEST_STD_STRING_VIEW
	// #define TEST_CXX20_STRING_VIEW_FEATURES
	using std::string_view;
#else
	#define TEST_CXX20_STRING_VIEW_FEATURES
	using fastcws::rep_aware::string_view;
#endif

TEST(string_view, default_ctor) {
	const string_view s;
	EXPECT_EQ(s.data(), nullptr);
	EXPECT_TRUE(s.empty());
}

TEST(string_view, cstr_ctor) {
	const char data[] = "hello";
	const string_view s(data);
	EXPECT_EQ(s.data(), data);
	EXPECT_EQ(s.size(), 5);
}

TEST(string_view, data_len_ctor) {
	const char data[] = "hello";
	const string_view s(data, 3);
	EXPECT_EQ(s.data(), data);
	EXPECT_EQ(s.size(), 3);
}

TEST(string_view, string_ctor) {
	const std::string data("hello\0world", 11);
	const string_view s(data);
	EXPECT_EQ(s.data(), data.data());
	EXPECT_EQ(s.size(), data.size());
}

TEST(string_view, cstr_assignment) {
	const char data[] = "hello";
	const string_view s = data;
	EXPECT_EQ(s.data(), data);
	EXPECT_EQ(s.size(), 5);
}

TEST(string_view, assignment) {
	const string_view t = "hello";
	const string_view s(t);
	EXPECT_EQ(s.data(), t.data());
	EXPECT_EQ(s.size(), t.size());
}

TEST(string_view, iterators) {
	const string_view s = "hello";
	EXPECT_EQ(*s.begin(), 'h');
	EXPECT_EQ(*(s.end() - 1), 'o');
	EXPECT_EQ(*s.cbegin(), 'h');
	EXPECT_EQ(*(s.cend() - 1), 'o');
	EXPECT_EQ(*s.rbegin(), 'o');
	EXPECT_EQ(*(s.rend() - 1), 'h');
	EXPECT_EQ(*s.crbegin(), 'o');
	EXPECT_EQ(*(s.crend() - 1), 'h');
}

TEST(string_view, size) {
	string_view s = "hello";
	EXPECT_EQ(s.size(), 5);
	EXPECT_EQ(s.size(), s.length());
	EXPECT_FALSE(s.empty());
	s = "";
	EXPECT_EQ(s.size(), s.length());
	EXPECT_TRUE(s.empty());
}

TEST(string_view, access) {
	const string_view s = "hello world!";
	EXPECT_EQ(s[0], 'h');
	EXPECT_EQ(s.at(0), 'h');
	EXPECT_EQ(s[s.size() - 1], '!');
	EXPECT_EQ(s.at(s.size() - 1), '!');
	EXPECT_EQ(s.front(), 'h');
	EXPECT_EQ(s.back(), '!');
}

TEST(string_view, at_out_of_range) {
	const string_view s = "hello";
	try {
		s.at(100);
		FAIL() << "throw expected";
	} catch (const std::out_of_range& e) {
		(void)e;
	} catch (...) {
		FAIL() << "std::out_of_range expected";
	}
}

TEST(string_view, max_size) {
	const string_view s = "hello";
	EXPECT_GT(s.max_size(), 0);
}

TEST(string_view, remove_prefix) {
	const char data[] = "hello world!";
	constexpr string_view::size_type offset = 6;           /* size of "hello " */
	constexpr string_view::size_type remaining_offset = 6; /* size of "world!" */
	string_view s = data;
	s.remove_prefix(offset);
	EXPECT_EQ(s.size(), remaining_offset);
	EXPECT_EQ(s.data(), data + offset);
	s.remove_prefix(remaining_offset);
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.data(), data + offset + remaining_offset);
}

TEST(string_view, remove_suffix) {
	const char data[] = "hola mundo!";
	constexpr string_view::size_type offset = 6;           /* size of "mundo!" */
	constexpr string_view::size_type remaining_offset = 5; /* size of "hola " */
	string_view s = data;
	s.remove_suffix(offset);
	EXPECT_EQ(s.size(), remaining_offset);
	EXPECT_EQ(s.data(), data);
	s.remove_suffix(remaining_offset);
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.data(), data);
}

TEST(string_view, remove_prefix_and_suffix) {
	const char data[] = "hello mundo!";
	constexpr string_view::size_type offset = 6;           /* size of "mundo!" */
	constexpr string_view::size_type remaining_offset = 6; /* size of "hello " */
	string_view s = data;
	s.remove_suffix(offset);
	EXPECT_EQ(s.size(), remaining_offset);
	EXPECT_EQ(s.data(), data);
	s.remove_prefix(remaining_offset);
	EXPECT_TRUE(s.empty());
	EXPECT_EQ(s.data(), data + remaining_offset);
}

TEST(string_view, swap) {
	const char data_s[] = "hola";
	const char data_t[] = "mundo";
	string_view s = data_s;
	string_view t = data_t;
	s.swap(t);
	EXPECT_EQ(s.data(), data_t);
	EXPECT_EQ(s.size(), 5);
	EXPECT_EQ(t.data(), data_s);
	EXPECT_EQ(t.size(), 4);
	string_view empty;
	t.swap(empty);
	EXPECT_TRUE(t.empty());
}

TEST(string_view, copy) {
	const string_view s = "hola";
	char data[5];
	// copy the whole string
	std::fill(std::begin(data), std::end(data), 0);
	EXPECT_EQ(s.copy(data, 4), 4);
	EXPECT_EQ(strcmp(data, "hola"), 0);
	// copy a substring
	std::fill(std::begin(data), std::end(data), 0);
	EXPECT_EQ(s.copy(data, 4, 2), 2);
	EXPECT_EQ(strcmp(data, "la"), 0);
	// copy with a count bigger than the string's length
	std::fill(std::begin(data), std::end(data), 0);
	EXPECT_EQ(s.copy(data, 5), 4);
	EXPECT_EQ(strcmp(data, "hola"), 0);
	// copy an empty string
	std::fill(std::begin(data), std::end(data), 0);
	EXPECT_EQ(s.copy(data, 4, 4), 0);
	EXPECT_EQ(strcmp(data, ""), 0);
}

TEST(string_view, substr) {
	const string_view s = "string view test";
	EXPECT_EQ(s.substr(), "string view test");
	EXPECT_EQ(s.substr(2), "ring view test");
	EXPECT_EQ(s.substr(2, 3), "rin");
	EXPECT_TRUE(s.substr(16, 1).empty());
}

TEST(string_view, compare) {
	{
		const string_view s = "hello";
		EXPECT_LT(s.compare(string_view("hillo")), 0);
		EXPECT_LT(s.compare(string_view("hello there")), 0);
		EXPECT_LT(s.compare("hola"), 0);
		EXPECT_LT(s.compare("hello!"), 0);
	}

	{
		const string_view s = "world";
		EXPECT_GT(s.compare(string_view("hello")), 0);
		EXPECT_GT(s.compare(string_view("wo")), 0);
		EXPECT_GT(s.compare("wonderland"), 0);
		EXPECT_GT(s.compare(""), 0);
	}
	{
		const string_view s = "hello";
		EXPECT_EQ(s.compare(string_view("hello")), 0);
		EXPECT_EQ(s.compare("hello"), 0);
	}
}

TEST(string_view, compare_substr) {
	{
		const string_view s = "hello";
		EXPECT_LT(s.compare(1, 2, string_view("il")), 0);
		EXPECT_LT(s.compare(2, 3, string_view("llo there")), 0);
		EXPECT_LT(s.compare(0, 3, "hi"), 0);
		EXPECT_LT(s.compare(0, 5, "hello there"), 0);
	}
	{
		const string_view s = "what a wonderful hello world";
		EXPECT_GT(s.compare(4, 3, string_view("   ")), 0);
		EXPECT_GT(s.compare(7, 9, string_view("wonder")), 0);
		EXPECT_GT(s.compare(1, 3, "has"), 0);
		EXPECT_GT(s.compare(s.size() - 5, 5, "w"), 0);
	}
	{
		const string_view s = "abc";
		EXPECT_EQ(s.compare(1, 2, string_view("bc")), 0);
		EXPECT_EQ(s.compare(0, 2, string_view("ab")), 0);
	}
}

TEST(string_view, compare_substr_substr) {
	{
		const string_view s = "substring";
		EXPECT_LT(s.compare(3, 6, string_view("strong wind"), 0, 6), 0);
		EXPECT_LT(s.compare(5, 4, string_view("ringo starr"), 0, 5), 0);
		EXPECT_LT(s.compare(0, 2, "ttss", 1, 2), 0);
		EXPECT_LT(s.compare(0, 2, "best subaru", 5, 6), 0);
	}
	{
		const string_view s = "https://youtu.be/dQw4w9WgXcQ";
		EXPECT_GT(s.compare(8, 8, string_view(" https://youtu.ae"), 1, 16), 0);
		EXPECT_GT(s.compare(0, 5, string_view("the http"), 4, 4), 0);
		EXPECT_GT(s.compare(0, 2, "hello", 1, 1), 0);
		EXPECT_GT(s.compare(1, 2, "star", 1, 1), 0);
	}
	{
		const string_view s = "linux";
		EXPECT_EQ(s.compare(2, 2, string_view("gnu"), 1, 2), 0);
		EXPECT_EQ(s.compare(0, 2, string_view("linus"), 0, 2), 0);
	}
}

TEST(string_view, compare_with_zeroes) {
	const std::string data_s("hello\0world", 11);
	const string_view s(data_s);
	const std::string data_t("hello\0friend", 12);
	EXPECT_GT(s.compare(string_view(data_t)), 0);
}

#ifdef TEST_CXX20_STRING_VIEW_FEATURES

TEST(string_view, starts_with) {
	const string_view s = "some text";
	EXPECT_TRUE(s.starts_with(string_view("some")));
	EXPECT_TRUE(s.starts_with('s'));
	EXPECT_TRUE(s.starts_with("some"));
	EXPECT_FALSE(s.starts_with(string_view("not")));
	EXPECT_FALSE(s.starts_with('t'));
	EXPECT_FALSE(s.starts_with("not"));
	EXPECT_FALSE(s.starts_with("some text that is too large"));
}

TEST(string_view, ends_with) {
	const string_view s = "some text";
	EXPECT_TRUE(s.ends_with(string_view("text")));
	EXPECT_TRUE(s.ends_with('t'));
	EXPECT_TRUE(s.ends_with("text"));
	EXPECT_FALSE(s.ends_with(string_view("not")));
	EXPECT_FALSE(s.ends_with('s'));
	EXPECT_FALSE(s.ends_with("not"));
	EXPECT_FALSE(s.ends_with("too large some text"));
}

#endif

TEST(string_view, find) {
	{
		const string_view s = "pattern here";
		// pos > haystack.size()
		EXPECT_EQ(s.find(string_view("pattern"), 30), string_view::npos);
		EXPECT_EQ(s.find('p', 30), string_view::npos);
		EXPECT_EQ(s.find("pattern", 30, 7), string_view::npos);
		EXPECT_EQ(s.find("pattern", 30), string_view::npos);
		// needle.size() > haystack.size()
		EXPECT_EQ(s.find(string_view("pattern here and there")), string_view::npos);
		EXPECT_EQ(s.find(string_view("pattern here"), 1), string_view::npos);
		EXPECT_EQ(s.find("pattern here", 1), string_view::npos);
		EXPECT_EQ(s.find("pattern here", 1, 12), string_view::npos);
		// not found
		EXPECT_EQ(s.find(string_view("Pattern here")), string_view::npos);
		EXPECT_EQ(s.find('z'), string_view::npos);
		EXPECT_EQ(s.find('p', 1), string_view::npos);
		EXPECT_EQ(s.find("Pattern"), string_view::npos);
		EXPECT_EQ(s.find("attern", 2), string_view::npos);
		EXPECT_EQ(s.find("ttern", 3, 5), string_view::npos);
	}
	{
		const string_view s = "pattern here pattern there, the end";
		// found
		EXPECT_EQ(s.find(string_view("pattern")), 0);
		EXPECT_EQ(s.find('a'), 1);
		EXPECT_EQ(s.find('a', 2), 14);
		EXPECT_EQ(s.find("pattern", 1), 13);
		EXPECT_EQ(s.find("end", 1, 3), s.size() - 3);
		EXPECT_EQ(s.find("end is near", 1, 3), s.size() - 3);
		EXPECT_EQ(s.find("", s.size()), s.size());
	}
}

TEST(string_view, find_empty) {
	const string_view empty;
	EXPECT_EQ(empty.find('a'), string_view::npos);
	EXPECT_EQ(empty.find('a', 1), string_view::npos);
	EXPECT_EQ(empty.find(""), 0);
	EXPECT_EQ(empty.find("", 1), string_view::npos);

	const string_view s = "pattern";
	EXPECT_EQ(s.find(""), 0);
	EXPECT_EQ(s.find("", 1), 1);
}

TEST(string_view, rfind) {
	{
		const string_view s = "pattern here";
		// not found
		EXPECT_EQ(s.rfind(string_view("pattern herE")), string_view::npos);
		EXPECT_EQ(s.rfind('q'), string_view::npos);
		EXPECT_EQ(s.rfind('a', 0), string_view::npos);
		EXPECT_EQ(s.rfind("patterN"), string_view::npos);
		EXPECT_EQ(s.rfind("Attern", 1), string_view::npos);
		EXPECT_EQ(s.rfind("ttern", 1, 5), string_view::npos);
	}
	{
		const string_view s = "pattern here pattern there, the end";
		// found
		EXPECT_EQ(s.rfind(string_view("pattern")), 13);
		EXPECT_EQ(s.rfind('a'), 14);
		EXPECT_EQ(s.rfind('a', 2), 1);
		EXPECT_EQ(s.rfind("pattern", 12), 0);
		EXPECT_EQ(s.rfind("end", s.size() - 2, 3), s.size() - 3);
		EXPECT_EQ(s.rfind("end is near", s.size() - 2, 3), s.size() - 3);
	}
}

TEST(string_view, rfind_empty) {
	const string_view empty;
	EXPECT_EQ(empty.rfind('a'), string_view::npos);
	EXPECT_EQ(empty.rfind('a', 1), string_view::npos);
	EXPECT_EQ(empty.rfind(""), 0);
	EXPECT_EQ(empty.rfind("", 1), 0);

	const string_view s = "pattern";
	EXPECT_EQ(s.rfind(""), s.size());
	EXPECT_EQ(s.rfind("", s.size() - 2), s.size() - 2);
}

TEST(string_view, find_first_of) {
	const string_view s = "pattern";
	// not found
	EXPECT_EQ(s.find_first_of(string_view("xyz")), string_view::npos);
	EXPECT_EQ(s.find_first_of(string_view("n"), 7), string_view::npos);
	EXPECT_EQ(s.find_first_of('z'), string_view::npos);
	EXPECT_EQ(s.find_first_of('n', 7), string_view::npos);
	EXPECT_EQ(s.find_first_of("aer", 6, 3), string_view::npos);
	EXPECT_EQ(s.find_first_of("aer", 6), string_view::npos);
	// found
	EXPECT_EQ(s.find_first_of(string_view("ate")), 1);
	EXPECT_EQ(s.find_first_of(string_view("e"), 2), 4);
	EXPECT_EQ(s.find_first_of('a'), 1);
	EXPECT_EQ(s.find_first_of('n', 2), 6);
	EXPECT_EQ(s.find_first_of("rnt", 2, 2), 5);
	EXPECT_EQ(s.find_first_of("rnt", 2), 2);
}

TEST(string_view, find_last_of) {
	const string_view s = "pattern";
	// not found
	EXPECT_EQ(s.find_last_of(string_view("xyz")), string_view::npos);
	EXPECT_EQ(s.find_last_of(string_view("n"), 5), string_view::npos);
	EXPECT_EQ(s.find_last_of('z'), string_view::npos);
	EXPECT_EQ(s.find_last_of('n', 5), string_view::npos);
	EXPECT_EQ(s.find_last_of("tea", 1, 2), string_view::npos);
	EXPECT_EQ(s.find_last_of("te", 1), string_view::npos);
	// found
	EXPECT_EQ(s.find_last_of(string_view("ate")), 4);
	EXPECT_EQ(s.find_last_of(string_view("p"), 2), 0);
	EXPECT_EQ(s.find_last_of('t'), 3);
	EXPECT_EQ(s.find_last_of('t', 2), 2);
	EXPECT_EQ(s.find_last_of("pa", 1, 1), 0);
	EXPECT_EQ(s.find_last_of("rnt", 2), 2);

	const string_view empty;
	EXPECT_EQ(empty.find_last_of(string_view("")), string_view::npos);
}

TEST(string_view, find_first_not_of) {
	const string_view s = "pattern";
	// not found
	EXPECT_EQ(s.find_first_not_of(string_view("patern")), string_view::npos);
	EXPECT_EQ(s.find_first_not_of(string_view("tern"), 2), string_view::npos);
	EXPECT_EQ(s.find_first_not_of('n', 6), string_view::npos);
	EXPECT_EQ(s.find_first_not_of("paternz", 1, 6), string_view::npos);
	EXPECT_EQ(s.find_first_not_of("rn", 5), string_view::npos);
	// found
	EXPECT_EQ(s.find_first_not_of(string_view("ate")), 0);
	EXPECT_EQ(s.find_first_not_of(string_view("t"), 2), 4);
	EXPECT_EQ(s.find_first_not_of('t'), 0);
	EXPECT_EQ(s.find_first_not_of('t', 2), 4);
	EXPECT_EQ(s.find_first_not_of("pa", 1, 1), 1);
	EXPECT_EQ(s.find_first_not_of("rnt", 2), 4);

	const string_view empty;
	EXPECT_EQ(empty.find_first_not_of(string_view("")), string_view::npos);
}

TEST(string_view, find_last_not_of) {
	const string_view s = "pattern";
	// not found
	EXPECT_EQ(s.find_last_not_of(string_view("patern")), string_view::npos);
	EXPECT_EQ(s.find_last_not_of(string_view("pat"), 2), string_view::npos);
	EXPECT_EQ(s.find_last_not_of('p', 0), string_view::npos);
	EXPECT_EQ(s.find_last_not_of("paternz", 6, 6), string_view::npos);
	EXPECT_EQ(s.find_last_not_of("pa", 1), string_view::npos);
	// found
	EXPECT_EQ(s.find_last_not_of(string_view("enr")), 3);
	EXPECT_EQ(s.find_last_not_of(string_view("rn"), 6), 4);
	EXPECT_EQ(s.find_last_not_of('t'), 6);
	EXPECT_EQ(s.find_last_not_of('t', 2), 1);
	EXPECT_EQ(s.find_last_not_of("rn", 6, 1), 6);
	EXPECT_EQ(s.find_last_not_of("rnt", 2), 1);

	const string_view empty;
	EXPECT_EQ(empty.find_last_not_of(string_view("")), string_view::npos);
}

TEST(string_view, eq) {
	const string_view a = "hello";
	EXPECT_TRUE(a != string_view("hola"));
	EXPECT_FALSE(a == string_view("hola"));
	EXPECT_TRUE(a != string_view(a.data(), a.size() - 1));
	EXPECT_FALSE(a == string_view(a.data(), a.size() - 1));
	EXPECT_TRUE(a != string_view("hellp"));
	EXPECT_FALSE(a == string_view("hellp"));
	EXPECT_EQ(a, string_view("hello world", 5));
	EXPECT_EQ(a, string_view("hello"));
	EXPECT_FALSE(a != string_view("hello world", 5));
	EXPECT_FALSE(a != string_view("hello"));
}

TEST(string_view, lt) {
	const string_view a = "hello";
	EXPECT_TRUE(a < string_view("hellp"));
	EXPECT_TRUE(a < string_view("hello world"));
	EXPECT_FALSE(string_view("hellp") < a);
	EXPECT_FALSE(string_view("hello") < a);
}

TEST(string_view, gt) {
	const string_view a = "hellp";
	EXPECT_TRUE(a > string_view("hello"));
	EXPECT_TRUE(a > string_view("hell"));
	EXPECT_FALSE(string_view("hello") > a);
	EXPECT_FALSE(string_view("hell") > a);
}

TEST(string_view, lte) {
	const string_view a = "hello";
	EXPECT_TRUE(a <= string_view("hellp"));
	EXPECT_TRUE(a <= string_view("hello world"));
	EXPECT_TRUE(a <= string_view("hello"));
	EXPECT_FALSE(string_view("hellp") <= a);
	EXPECT_FALSE(string_view("hello world") <= a);
	EXPECT_TRUE(string_view("hello") <= a);
}

TEST(string_view, gte) {
	const string_view a = "hello";
	EXPECT_TRUE(a >= string_view("hell"));
	EXPECT_TRUE(a >= string_view("a"));
	EXPECT_TRUE(a >= string_view("hello"));
	EXPECT_FALSE(string_view("hell") >= a);
	EXPECT_FALSE(string_view("a") >= a);
	EXPECT_TRUE(string_view("hello") >= a);
}

TEST(string_view, ostream_output) {
	const string_view a = "hello world";
	std::ostringstream oss;
	oss << a;
	EXPECT_EQ(oss.str(), "hello world");

	oss.str("");
	oss << std::setw(20) << std::left << a;
	EXPECT_EQ(oss.str(), "hello world         ");

	oss.str("");
	oss << std::setw(20) << std::right << a;
	EXPECT_EQ(oss.str(), "         hello world");

	oss.str("");
	oss << std::setw(20) << std::setfill('*') << std::left << a;
	EXPECT_EQ(oss.str(), "hello world*********");

	oss.str("");
	oss << std::setw(20) << std::setfill('*') << std::right << a;
	EXPECT_EQ(oss.str(), "*********hello world");

	oss.str("");
	oss << std::setw(20) << std::setfill('*') << std::right << a << a;
	EXPECT_EQ(oss.str(), "*********hello worldhello world");
}

TEST(string_view, hash) {
	EXPECT_EQ(std::hash<string_view>{}("hello world"), std::hash<std::string>{}("hello world"));
}

