cmake_minimum_required(VERSION 3.10)
project(client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(COMMON_LIB_DIR ${COMMON_DIR}/lib)
set(COMMON_INCLUDE_DIR ${COMMON_DIR}/include)

file(GLOB_RECURSE ALL_SOURCES "*.cpp")

list(FILTER ALL_SOURCES EXCLUDE REGEX "CMakeFiles/.*$")

add_executable(client ${ALL_SOURCES})

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES
    IMPORTED_LOCATION ${COMMON_LIB_DIR}/libcommon.a
    INTERFACE_INCLUDE_DIRECTORIES ${COMMON_INCLUDE_DIR}
)

if(NOT EXISTS ${COMMON_LIB_DIR}/libcommon.a)
    message(WARNING "Common library not found at ${COMMON_LIB_DIR}/libcommon.a")
    message(WARNING "Please build common library first:
    cd ../common
    make")
endif()

target_link_libraries(client PRIVATE common)

target_include_directories(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMMON_INCLUDE_DIR}
)