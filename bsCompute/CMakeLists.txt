project(bsCompute)

include_directories(../burning_ship)
include_directories(../colormaps)
include_directories(../renders)


set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)


find_package(Boost COMPONENTS REQUIRED json)

if(NOT DEFINED BOOST_INCLUDE_DIR)
    set(BOOST_INCLUDE_DIR "/home/jeremiah/gcc/gcc12-native/include")
endif()

message("BOOST_INCLUDE_DIR = " ${BOOST_INCLUDE_DIR})
include_directories(${BOOST_INCLUDE_DIR})


#add_executable(bsCompute 
#    userinput.h
#    userinput.cpp
#    bsCompute.cpp)

add_executable(bsTaskGen
    userinput.h
    userinput.cpp
    bsTaskGen.cpp)

add_executable(bsTaskRun 
    userinput.h
    userinput.cpp
    bsTaskRun.cpp)

#target_link_libraries(bsCompute PRIVATE burning_ship_lib colormaps burning_ship_render Boost::json)


target_link_libraries(bsTaskGen PRIVATE burning_ship_lib colormaps burning_ship_render Boost::json)
install(TARGETS bsTaskGen
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
target_link_libraries(bsTaskRun PRIVATE burning_ship_lib colormaps burning_ship_render Boost::json)
install(TARGETS bsTaskRun
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

#install(TARGETS bsCompute
#    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})