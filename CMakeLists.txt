cmake_minimum_required(VERSION 3.10)
project(Maxima)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

add_executable(
        Maxima
        function_maxima.h
        main.cpp
        #        example.cpp
        #        maximaTest.cpp
        #        wyjatkowy_int.cpp
)

target_link_libraries(Maxima ${GTEST_LIBRARIES} pthread)