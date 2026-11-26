cmake_minimum_required(VERSION 3.19)

project(Libertad1OrbitalGuardian VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# Cada vez que escribas una clase nueva, agrega aquí su .h y su .cpp
qt_add_executable(libertad1 WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp

    src/core/inventory.h src/core/inventory.cpp
    src/core/constants.h
    src/core/vector2d.h src/core/vector2d.cpp
    src/core/orbitalphysics.h src/core/orbitalphysics.cpp



)

target_include_directories(libertad1 PRIVATE src)
target_link_libraries(libertad1 PRIVATE Qt6::Widgets)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libertad1 PRIVATE -Wall -Wextra -Wpedantic)
endif()
