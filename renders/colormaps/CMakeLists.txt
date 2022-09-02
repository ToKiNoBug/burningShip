cmake_minimum_required(VERSION 3.0.0)
#project(burningShip VERSION 0.1.0)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

add_library(colormaps 
    STATIC 
    colors.h 
    colors.c
    color_sources.h 
    color_sources.c)