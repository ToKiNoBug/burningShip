project(bsCompute)

include_directories(../burning_ship)
include_directories(../colormaps)
include_directories(../renders)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)


find_package(Boost COMPONENTS REQUIRED json)

include_directories(${BOOST_INCLUDE_DIR})

add_executable(bsCompute 

    userinput.h
    userinput.cpp
    bsCompute.cpp)

target_link_libraries(bsCompute PRIVATE burning_ship_lib colormaps burning_ship_render Boost::json)


install(TARGETS bsCompute
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})