project(bsCompute)

include_directories(../burning_ship)
include_directories(../colormaps)
include_directories(../renders)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

add_executable(bsCompute 

    userinput.h
    userinput.cpp
    bsCompute.cpp)

target_link_libraries(bsCompute PRIVATE burning_ship_lib colormaps burning_ship_render)