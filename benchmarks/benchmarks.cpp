#include "string.hpp"

#include <ext/vstring.h>

#include <chrono>
#include <iostream>
#include <string>

template <typename Duration>
class timer {
	typedef std::chrono::high_resolution_clock clock;
public:
	void reset() {
		m_start = clock::now();
	}

	typename Duration::rep elapsed() const {
		return std::chrono::duration_cast<Duration>(clock::now() - m_start).count();
	}

private:
	clock::time_point m_start = clock::now();
};

template <typename T>
void test() {
	auto test_string = "0123456789"
	                   "0123456789"
	                   "0123456789"
	                   "0123456789"
	                   "0123456789";
	auto const test_length = 20;
	auto const reps = 1000000;

	std::cout << " (" << sizeof(T) << " bytes)" << std::endl;
	for(int length = 0; length < test_length; ++length)
	{
		std::cout << std::endl << "Length " << length << ": ";
		timer<std::chrono::microseconds> timer;

		std::vector<T> strings;
		strings.reserve(reps);

		// Construction
		timer.reset();
		for(int i = 0; i < reps; ++i) {
			strings.emplace_back(test_string, length);
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << "Construction: " << time_taken << "us, ";
		}

		// Size
		{
			auto count = 0u;
			timer.reset();
			for(int i = 0; i < reps; ++i) {
				count += strings.size();
			}
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << " Size: " << time_taken << "us, ";
		}

		// Capacity
		{
			auto count = 0u;
			timer.reset();
			for(int i = 0; i < reps; ++i) {
				count += strings.capacity();
			}
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << " Capacity: " << time_taken << "us, ";
		}

		// Data
		{
			auto count = 0u;
			timer.reset();
			for(int i = 0; i < reps; ++i) {
				count += *reinterpret_cast<unsigned char*>(strings.data());
			}
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << " Data: " << time_taken << "us, ";
		}

		// Copying
		{
			auto count = 0u;
			timer.reset();
			for(int i = 0; i < reps; ++i) {
				auto copy = strings[0];
				count += copy.size();
			}
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << " Copy: " << time_taken << "us, ";
		}

		// Moving
		{
			auto count = 0u;
			timer.reset();
			for(int i = 0; i < reps; ++i) {
				auto move = std::move(strings[0]);
				count += move.size();
				strings[0] = std::move(move);
			}
		}

		{
			auto time_taken = timer.elapsed();
			std::cout << " Move: " << time_taken << "us, ";
		}
	}
}

int main() {
	typedef __gnu_cxx::__versa_string<char> vstring;

	std::cout << "sso23::string";
	test<sso23::string>();
	std::cout << std::endl;

	std::cout << "std::string";
	test<std::string>();
	std::cout << std::endl;

	std::cout << "vstring";
	test<vstring>();
	std::cout << std::endl;
}
