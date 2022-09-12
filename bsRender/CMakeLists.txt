cmake_minimum_required(VERSION 3.0.0)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

include_directories(../burning_ship)
include_directories(../colormaps)
include_directories(../renders)


find_package(Boost COMPONENTS REQUIRED json)
find_package(OpenMP REQUIRED)

include_directories(${BOOST_INCLUDE_DIR})

add_executable(bsRender  
    
    userinput_render.h
    userinput_render.cpp
    bsRender.cpp 
    execute_task.cpp)

    

target_link_libraries(bsRender 
    PRIVATE 
    
    burning_ship_lib 
    colormaps 
    burning_ship_render 
    Boost::json 
    OpenMP::OpenMP_CXX)


install(TARGETS bsRender
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})