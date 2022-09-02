cmake_minimum_required(VERSION 3.0.0)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH D:/Qt/6.1.0/gcc12-static)
set(QT_DIR ${CMAKE_PREFIX_PATH}/lib/cmake/Qt6)
set(QT_QMAKE_EXECUTABLE ${CMAKE_PREFIX_PATH}/bin/qmake.exe)

include_directories(../burning_ship)
include_directories(../colormaps)
include_directories(../renders)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(zoomer_header_files 
 
    zoomer.h
    scalable_label.h)

set(zoomer_source_files 
    
    main.cpp
    zoomer.cpp
    scalable_label.cpp)

set(zoomer_ui_files 
    
    zoomer.ui)

set(zoomer_project_sources
    ${zoomer_header_files} 
    ${zoomer_source_files} 
    ${zoomer_ui_files})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(zoomer 
        MANUAL_FINALIZATION
        ${zoomer_project_sources})
endif()

target_link_libraries(zoomer
    PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    burning_ship_lib 
    burning_ship_render 
    )

set_target_properties(zoomer PROPERTIES
    WIN32_EXECUTABLE TRUE)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(zoomer)
endif()
