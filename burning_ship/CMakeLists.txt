cmake_minimum_required(VERSION 3.0.0)
#project(burningShip VERSION 0.1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)



find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(OpenMP REQUIRED)


include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

add_library(burning_ship_lib STATIC 
    burning_ship.h 
    compute_frame.c 
    write_frame.c 
    render_frame.c 
    frame_operations.c)

target_link_libraries(burning_ship_lib OpenMP::OpenMP_C png z quadmath)