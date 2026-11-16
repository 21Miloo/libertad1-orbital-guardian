cmake_minimum_required(VERSION 3.16)

project(OrbitalGuardian VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

qt_add_executable(OrbitalGuardian
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(OrbitalGuardian PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(OrbitalGuardian PROPERTIES
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS OrbitalGuardian
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(OrbitalGuardian)
