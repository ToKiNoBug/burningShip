cmake_minimum_required(VERSION 3.0.0)
#project(burningShip VERSION 0.1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)



find_package(ZLIB 1.2.11 REQUIRED)
#find_package(PNG REQUIRED)
find_package(OpenMP REQUIRED)

#find_package(quadmath)


if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#add_compile_options("-Ofast")

endif()


include_directories(${ZLIB_INCLUDE_DIR})
#include_directories(${PNG_INCLUDE_DIR})

add_library(burning_ship_lib STATIC 
    burning_ship.h 
    compute_frame.c 
    write_frame.c 
    render_frame.c 
    frame_operations.c
    assembly_defines.h 
    assembly_impl.c)

target_link_libraries(burning_ship_lib OpenMP::OpenMP_C z quadmath)

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_options(burning_ship_lib PRIVATE -Ofast)
endif()

target_compile_options(burning_ship_lib PUBLIC -mavx -mavx2)