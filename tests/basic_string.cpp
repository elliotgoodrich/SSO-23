#include "string.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm>

BOOST_AUTO_TEST_SUITE(basic_string)

BOOST_AUTO_TEST_CASE(Constructor) {

	auto test_string = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::size_t test_size = 10;

	auto i = test_size;
	do {
		auto const string_length = test_size - i;
		sso23::string str{test_string, string_length};
		BOOST_CHECK_EQUAL(str.size(), string_length);

		auto const sso_capacity = sso23::string::sso_capacity;
		auto const capacity = std::max(sso_capacity, string_length);
		BOOST_CHECK_EQUAL(str.capacity(), capacity);
		BOOST_CHECK_EQUAL(str, test_string + i);
	} while(i-- != 0);
}

BOOST_AUTO_TEST_SUITE_END()
