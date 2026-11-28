cmake_minimum_required(VERSION 3.19)

project(Libertad1OrbitalGuardian VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
qt_standard_project_setup()

# Cada vez que escribas una clase nueva, agrega aquí su .h y su .cpp
qt_add_executable(libertad1 WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp

    src/core/inventory.h src/core/inventory.cpp
    src/core/constants.h
    src/core/difficulty.h
    src/core/vector2d.h src/core/vector2d.cpp
    src/core/orbitalphysics.h src/core/orbitalphysics.cpp
    src/orbitwindow.h src/orbitwindow.cpp

    src/objects/spaceobject.h src/objects/spaceobject.cpp
    src/objects/projectile.h src/objects/projectile.cpp
    src/objects/debris.h src/objects/debris.cpp
    src/objects/drone.h src/objects/drone.cpp

    src/agent/soundwaveagent.h src/agent/soundwaveagent.cpp

    src/audio/gameaudio.h src/audio/gameaudio.cpp
)

# Audio embebido en el .exe: sin rutas externas que se rompan al distribuir.
# GLOB no entra a fuentes/ (originales pesados, fuera de git).
# BIG_RESOURCES: lo enlaza como objeto binario en vez de generar un .cpp gigante.
file(GLOB AUDIO_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/recursos/audio/*.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/recursos/audio/*.mp3")
qt_add_resources(libertad1 "audio"
    PREFIX "/audio"
    BASE "recursos/audio"
    BIG_RESOURCES
    FILES ${AUDIO_FILES})

target_include_directories(libertad1 PRIVATE src)
target_link_libraries(libertad1 PRIVATE Qt6::Widgets Qt6::Multimedia)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libertad1 PRIVATE -Wall -Wextra -Wpedantic)
endif()
