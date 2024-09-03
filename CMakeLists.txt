cmake_minimum_required(VERSION 3.25)
project(highlighter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory("${CMAKE_SOURCE_DIR}/zlib-develop")
add_subdirectory("${CMAKE_SOURCE_DIR}/libpng-libpng16")

add_executable(hl main.cpp tinyxml2.cpp)

target_include_directories(hl PRIVATE "${CMAKE_SOURCE_DIR}/libpng-libpng16")
target_link_libraries(hl png_shared)