cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


project(rocket
        DESCRIPTION "A light C++ RPC lib"
        LANGUAGES C CXX)

set(ROCKET_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include_directories(${ROCKET_SRC_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(tests)