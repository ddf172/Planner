cmake_minimum_required(VERSION 3.5)

project(server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NLOHMANN_JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../extern/nlohmann)

set(PROJECT_SOURCES
    main.cpp
    src/network/ServerSocket.cpp
    include/network/ServerSocket.hpp
    include/message/MessageFrame.hpp
    include/schedule/ScheduleData.hpp
)

add_executable(server ${PROJECT_SOURCES})

target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NLOHMANN_JSON_DIR}
)

set_target_properties(server PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)