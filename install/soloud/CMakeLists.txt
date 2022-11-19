cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED SDL2_DIR)
    message(FATAL_ERROR "SDL2_DIR must be defined")
endif()

project(soloud)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)

file(WRITE include/psp2/audioout.h "#pragma once")
file(WRITE include/psp2/kernel/threadmgr.h "#pragma once")

file(GLOB_RECURSE
    SOLOUD_SOURCES
    src/audiosource/*.cpp
    src/audiosource/*.h
    src/audiosource/*.c
    src/backend/*.cpp
    src/core/*.cpp
    include/*.h
)

add_library(soloud ${SOLOUD_SOURCES})

set_target_properties(
    soloud
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "../build"
)

link_directories(soloud ${SDL_HOME}/sdk/lib)

target_link_libraries(soloud SDL2::SDL2-static)

target_include_directories(soloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(soloud PRIVATE ${SDL2_INCLUDE_DIR})

file(GLOB_RECURSE SOLOUD_HEADERS include/*.h)
set_target_properties(soloud PROPERTIES PUBLIC_HEADER "${SOLOUD_HEADERS}")

add_compile_definitions(WITH_SDL2_STATIC)

install(
    TARGETS soloud
    EXPORT soloudTargets
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/soloud
)

install(
    EXPORT soloudTargets
    FILE soloudTargets.cmake
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    NAMESPACE soloud::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    soloudConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/soloudConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/soloudConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)