cmake_minimum_required(VERSION 3.0.0)
#project(burningShip VERSION 0.1.0)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

include_directories(../burning_ship)
include_directories(../colormaps)


find_package(OpenMP REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB 1.2.11 REQUIRED)


include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})


add_library(burning_ship_render 
    STATIC 
    
    renders.h 
    smooth_by_norm2.c
    coloring_by_f32.c
    write_png.c)

target_link_libraries(burning_ship_render 
    PRIVATE 
    
    OpenMP::OpenMP_C 
    z
    png
    
    burning_ship_lib 
    colormaps)