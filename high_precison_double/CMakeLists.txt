cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)

add_library(hpd STATIC 
    high_precision_double.h
    high_precision_double.cpp)

add_executable(test_hpd test_hpd.cpp)

target_link_libraries(hpd PRIVATE -lquadmath)
target_link_libraries(test_hpd PRIVATE -lquadmath)