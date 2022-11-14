cmake_minimum_required(VERSION 3.13)

include(base_lib/CMakeLists.txt)

project(hexa_engine)

set(CMAKE_CXX_STANDARD 23)



#hexa_engine
setup_dll(hexa_engine "src/*.cpp" "include/hexa_engine/*.h")

add_dependencies(hexa_engine base_lib)
target_link_libraries(hexa_engine base_lib)

target_include_directories(hexa_engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)