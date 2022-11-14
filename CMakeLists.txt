cmake_minimum_required(VERSION 3.13)

include(base_lib/CMakeLists.txt)

project(hexa_engine)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    add_compile_definitions(_WINDOWS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(_LINUX)
endif()

add_executable(hexa_engine main.cpp)