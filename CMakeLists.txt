cmake_minimum_required(VERSION 3.24)
project(automata)

set(CMAKE_CXX_STANDARD 11)

add_executable(automata main.cpp structures.cpp subset_construction.cpp minimizer.cpp)
