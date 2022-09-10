cmake_minimum_required(VERSION 3.0.0)
project(burningShip VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

#message("CMAKE_INSTALL_PREFIX = "${CMAKE_INSTALL_PREFIX})
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

if(EXISTS ${CMAKE_SOURCE_DIR}/compile_commands.json)
    file(REMOVE ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()


find_package(Boost COMPONENTS json)

if(NOT BOOST_FOUND) 

set(BOOST_ROOT D:/CppLibs/libBoost-1.79.0/lib/cmake)

find_package(Boost COMPONENTS REQUIRED json)

endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
# this macro is added since in C, inline functions may cause `undefined reference` error in debug mode.
add_compile_definitions(BS_Optimize_on)

endif()

add_compile_definitions(burning_ship_rows=2160)
add_compile_definitions(burning_ship_cols=3840)


add_subdirectory(colormaps)
add_subdirectory(burning_ship)
add_subdirectory(renders)
add_subdirectory(test_burning_ship)
add_subdirectory(zoomer)
add_subdirectory(bsCompute)