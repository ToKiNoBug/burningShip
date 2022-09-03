cmake_minimum_required(VERSION 3.0.0)
#project(burningShip VERSION 0.1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

include_directories(../burning_ship)
include_directories(../renders)

add_executable(test_burning_ship main.c)
target_link_libraries(test_burning_ship PRIVATE burning_ship_lib burning_ship_render )