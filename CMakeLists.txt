cmake_minimum_required(VERSION 3.13)



set(OGRE_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/ogre)
set(OIS_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/OIS)
set(RP3D_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/reactphysics3d)
set(SOLOUD_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/soloud)
set(OBJ_LOADER_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/OBJ-Loader)

set(OGRE_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/ogre/build/cmake")
find_package(OGRE REQUIRED COMPONENTS Bites RTShaderSystem CONFIG)

# include
#set(OGRE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party/ogre/cmake)
#find_package(OGRE REQUIRED COMPONENTS Bites RTShaderSystem CONFIG)

#if(NOT "${OGRE_VERSION_MAJOR}" STREQUAL "13")
#    message(SEND_ERROR "You need Ogre 13 to build this.")
#endif()

include(base_lib/CMakeLists.txt)

project(hexa_engine)

set(CMAKE_CXX_STANDARD 23)



# hexa_engine
setup_dll(hexa_engine "src/*.cpp" "include/hexa_engine/*.h")

add_dependencies(hexa_engine base_lib)
target_link_libraries(hexa_engine base_lib)
#link_directories(hexa_engine "${OGRE_HOME}/lib/Release" "${OGRE_HOME}/SDK/lib")
#link_directories(hexa_engine ${OIS_HOME}/sdk/lib)
#link_directories(hexa_engine ${RP3D_HOME}/sdk/lib)
#link_directories(hexa_engine ${SOLOUD_HOME}/sdk/lib)

target_include_directories(hexa_engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
#target_include_directories(hexa_engine PRIVATE ${OGRE_HOME}/SDK/include)
#target_include_directories(hexa_engine PRIVATE ${OIS_HOME}/sdk/include)
#target_include_directories(hexa_engine PRIVATE ${RP3D_HOME}/sdk/include)
#target_include_directories(hexa_engine PRIVATE ${SOLOUD_HOME}/sdk/include)
#target_include_directories(hexa_engine PRIVATE ${OBJ_LOADER_HOME}/Source)



# ogre
#file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})