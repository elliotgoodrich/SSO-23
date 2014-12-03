cmake_minimum_required(VERSION 2.6)

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror -std=c++11 -mwindows -g")
endif()

find_package(Boost 1.53.0 REQUIRED COMPONENTS unit_test_framework)

include_directories(../include/)
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCES
	basic_string.cpp
	main.cpp
)

add_executable(unit_tests ${SOURCES})
target_link_libraries(unit_tests ${Boost_LIBRARIES})
