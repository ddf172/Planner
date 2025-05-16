cmake_minimum_required(VERSION 3.10)
project(common VERSION 1.0.0 LANGUAGES CXX)

# Ustaw standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ustaw katalogi wyjściowe dla biblioteki
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Znajdź wszystkie pliki źródłowe
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Utwórz statyczną bibliotekę
add_library(common STATIC ${SOURCES} ${HEADERS})

# Dodaj katalogi include (project headers + nlohmann)
target_include_directories(common 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/nlohmann
)

# Konfiguracja instalacji biblioteki
install(TARGETS common
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Instalacja plików nagłówkowych
install(DIRECTORY include/ DESTINATION include)
