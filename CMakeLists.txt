cmake_minimum_required(VERSION 3.13)



set(OIS_HOME "C:/Program Files (x86)/OIS/")

# include
set(OGRE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/ogre/cmake)
find_package(OGRE REQUIRED COMPONENTS Bites RTShaderSystem CONFIG)

if(NOT "${OGRE_VERSION_MAJOR}" STREQUAL "13")
    message(SEND_ERROR "You need Ogre 13 to build this.")
endif()

include(base_lib/CMakeLists.txt)

project(hexa_engine)

set(CMAKE_CXX_STANDARD 23)



# hexa_engine
setup_dll(hexa_engine "src/*.cpp" "include/hexa_engine/*.h")

add_dependencies(hexa_engine base_lib)
target_link_libraries(hexa_engine base_lib)
target_link_libraries(hexa_engine ${OGRE_LIBRARIES})
target_link_libraries(hexa_engine ${OIS_HOME}/lib)

target_include_directories(hexa_engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(hexa_engine PUBLIC ${OGRE_INCLUDE_DIRS})
target_include_directories(hexa_engine PUBLIC ${OIS_HOME}/include)



# ogre
#file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})