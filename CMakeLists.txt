cmake_minimum_required(VERSION 3.13)

include(base_lib/CMakeLists.txt)

project(hexa_engine)

set(CMAKE_CXX_STANDARD 20)

file(GLOB HEXA_ENGINE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB HEXA_ENGINE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/hexa_engine/*.h)
add_library(hexa_engine ${HEXA_ENGINE_SOURCES} ${HEXA_ENGINE_HEADERS})

add_dependencies(hexa_engine base_lib)
target_include_directories(hexa_engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/base_lib/include)
target_link_libraries(hexa_engine base_lib)

set_output_directory(hexa_engine)