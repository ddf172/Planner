cmake_minimum_required(VERSION 3.5)

project(app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
set(COMMON_LIB_DIR ${COMMON_DIR}/build/lib)
set(COMMON_INCLUDE_DIR ${COMMON_DIR}/include)
set(COMMON_JSON_INCLUDE_DIR ${COMMON_DIR}/extern/nlohmann)


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        src/network/ClientSocket.cpp
        include/network/ClientSocket.hpp
)



if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(app
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET app APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(app SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(app
            ${PROJECT_SOURCES}
        )
    endif()
endif()

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES
        IMPORTED_LOCATION ${COMMON_LIB_DIR}/libcommon.a
        INTERFACE_INCLUDE_DIRECTORIES "${COMMON_INCLUDE_DIR};${COMMON_JSON_INCLUDE_DIR}"
)

if(NOT EXISTS ${COMMON_LIB_DIR}/libcommon.a)
    message(WARNING "Common library not found at ${COMMON_LIB_DIR}/libcommon.a")
    message(WARNING "Please build common library first:
    cd ../common
    make")
endif()


target_link_libraries(app PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Widgets
        common
)

set_target_properties(app PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(app)
endif()
