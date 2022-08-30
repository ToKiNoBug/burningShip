cmake_minimum_required(VERSION 3.0.0)
project(burningShip VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if(EXISTS ${CMAKE_SOURCE_DIR}/compile_commands.json)
    file(REMOVE ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()


find_package(zlib REQUIRED)
find_package(PNG REQUIRED)
find_package(OpenMP REQUIRED)


include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

add_executable(burningShip ship_iteration.h main.c compute_frame.c write_frame.c)

target_link_libraries(burningShip OpenMP::OpenMP_C png z)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
