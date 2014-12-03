cmake_minimum_required(VERSION 2.6)

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror -std=c++11 -mwindows")
endif()

include_directories(../include/)

set(SOURCES
	benchmarks.cpp
)

add_executable(benchmark ${SOURCES})
