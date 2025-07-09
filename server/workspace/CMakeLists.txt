cmake_minimum_required(VERSION 3.5)

project(server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(COMMON_LIB_DIR ${COMMON_DIR}/build/lib)
set(COMMON_INCLUDE_DIR ${COMMON_DIR}/include)
set(COMMON_JSON_INCLUDE_DIR ${COMMON_DIR}/extern/nlohmann)

set(PROJECT_SOURCES
    main.cpp
    src/network/ServerSocket.cpp
    include/network/ServerSocket.hpp
)

add_executable(server ${PROJECT_SOURCES})

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES
    IMPORTED_LOCATION ${COMMON_LIB_DIR}/libcommon.a
    INTERFACE_INCLUDE_DIRECTORIES "${COMMON_INCLUDE_DIR};${COMMON_JSON_INCLUDE_DIR}"
)

if(NOT EXISTS ${COMMON_LIB_DIR}/libcommon.a)
    message(WARNING "Common library not found at ${COMMON_LIB_DIR}/libcommon.a")
    message(WARNING "Please build common library first:")
    message(WARNING "    cd ../common")
    message(WARNING "    make")
endif()

target_link_libraries(server PRIVATE common)

target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COMMON_INCLUDE_DIR}
    ${COMMON_JSON_INCLUDE_DIR}
)

set_target_properties(server PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)