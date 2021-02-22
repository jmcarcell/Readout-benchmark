cmake_minimum_required (VERSION 3.0)

project(felix-long-readout-tools CXX)

add_executable(custom-dumpfile-to-text custom-dumpfile-to-text PdspChannelMapService.cpp hist.cpp)

SET(GCC_COVERAGE_COMPILE_FLAGS "-O3")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
