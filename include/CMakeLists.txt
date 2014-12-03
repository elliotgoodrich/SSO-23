cmake_minimum_required(VERSION 2.6)

include_directories(.)

set(SOURCES
	string.hpp
)

add_library(HEADER_ONLY_TARGET STATIC ${SOURCES})
set_target_properties(HEADER_ONLY_TARGET PROPERTIES LINKER_LANGUAGE CXX)
