cmake_minimum_required(VERSION 3.21)
project(yaparser)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_FLAGS_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

add_executable(yaparser main.cpp include/util.h include/LRAutomata.h include/LRAutomata.h)
