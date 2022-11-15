cmake_minimum_required(VERSION 3.13)

project(soloud)

set(CMAKE_CXX_STANDARD 23)

set(SDL_HOME "${CMAKE_CURRENT_LIST_DIR}/../SDL/")

file(WRITE ${CMAKE_CURRENT_LIST_DIR}/include/psp2/audioout.h "#pragma once")
file(WRITE ${CMAKE_CURRENT_LIST_DIR}/include/psp2/kernel/threadmgr.h "#pragma once")

file(GLOB_RECURSE SOLOUD_SOURCES1 ${CMAKE_CURRENT_LIST_DIR}/src/backend/*.cpp)
file(GLOB_RECURSE SOLOUD_SOURCES2 ${CMAKE_CURRENT_LIST_DIR}/src/core/*.cpp)
file(GLOB_RECURSE SOLOUD_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/*.h)

add_library(soloud "${SOLOUD_SOURCES1};${SOLOUD_SOURCES2}" ${SOLOUD_HEADERS})

set_target_properties(soloud
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE}"
        )

link_directories(soloud ${SDL_HOME}/sdk/lib)

target_include_directories(soloud PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(soloud PUBLIC ${SDL_HOME}/sdk/include/SDL2)

set_target_properties(soloud PROPERTIES PUBLIC_HEADER "${SOLOUD_HEADERS}")

add_compile_definitions(WITH_SDL2_STATIC)

install(TARGETS soloud
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/include/soloud")
#install(FILES ${SOLOUD_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")