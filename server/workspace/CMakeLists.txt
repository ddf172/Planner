cmake_minimum_required(VERSION 3.5)

project(server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../extern/nlohmann)

# Rekurencyjnie znajdź wszystkie pliki .cpp i .hpp
file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

add_executable(server ${PROJECT_SOURCES})

target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NLOHMANN_JSON_DIR}
)

# -----
# Add test client executable
add_executable(test_client test_client.cpp)

target_include_directories(test_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NLOHMANN_JSON_DIR}
)
# -----

set_target_properties(server PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)