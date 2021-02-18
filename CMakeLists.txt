cmake_minimum_required (VERSION 3.0)

project(felix-long-readout-tools CXX)

add_executable(custom-dumpfile-to-text custom-dumpfile-to-text PdspChannelMapService.cpp hist.cpp)
