cmake_minimum_required(VERSION 3.13)


include(base_lib/CMakeLists.txt)


set(RP3D_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/reactphysics3d)
set(SOLOUD_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/soloud)
set(OBJ_LOADER_HOME ${CMAKE_CURRENT_LIST_DIR}/third_party/OBJ-Loader)

# include
set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/SDL/sdk/${CMAKE_BUILD_TYPE}/cmake")
find_package(SDL2)

set(ZLIB_USE_STATIC_LIBS "ON")
if(DEBUG)
    set(ZLIB_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/third_party/zlib/sdk/${CMAKE_BUILD_TYPE}/lib/zlibstatic.lib")
else()
    set(ZLIB_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/third_party/zlib/sdk/${CMAKE_BUILD_TYPE}/lib/zlibstaticd.lib")
endif()
set(ZLIB_ROOT ${CMAKE_CURRENT_LIST_DIR}/third_party/zlib)
find_package(ZLIB)

#set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}/lib/vulkan-1.lib")
#find_package(Vulkan COMPONENTS shaderc_combined)

set(OGRE_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/ogre/SDK/${CMAKE_BUILD_TYPE}/cmake")
find_package(OGRE REQUIRED COMPONENTS Bites RTShaderSystem)

if(NOT "${OGRE_VERSION_MAJOR}" STREQUAL "13")
    message(SEND_ERROR "You need Ogre 13 to build this.")
endif()

project(hexa_engine)

set(CMAKE_CXX_STANDARD 23)



# hexa_engine
setup_dll(hexa_engine "src/*.cpp" "include/hexa_engine/*.h")

file(COPY_FILE "$ENV{VULKAN_SDK}/lib/shaderc_combined.lib" "${CMAKE_BINARY_DIR}/shaderc_combined.lib")
message(STATUS "copied shaderc_combined.lib")

find_library(pugixml_LIB pugixml HINTS ${OGRE_LIBRARY_DIRS})
file(COPY_FILE "${pugixml_LIB}" "${CMAKE_BINARY_DIR}/pugixml.lib")
message(STATUS "copied pugixml.lib")

add_dependencies(hexa_engine base_lib)
add_dependencies(hexa_engine OgreBites)
target_link_libraries(hexa_engine base_lib)
target_link_libraries(hexa_engine SDL2::SDL2-static)
target_link_libraries(hexa_engine ZLIB::ZLIB)
target_link_libraries(hexa_engine OgreBites)

link_directories("${OGRE_LIBRARY_DIRS}")
link_directories("${OIS_HOME}/sdk/${CMAKE_BUILD_TYPE}/lib")
link_directories("${RP3D_HOME}/sdk/${CMAKE_BUILD_TYPE}/lib")
link_directories("${SOLOUD_HOME}/sdk/${CMAKE_BUILD_TYPE}/lib")

target_include_directories(hexa_engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(hexa_engine PRIVATE ${OGRE_INCLUDE_DIRS})
target_include_directories(hexa_engine PRIVATE ${RP3D_HOME}/sdk/include)
target_include_directories(hexa_engine PRIVATE ${SOLOUD_HOME}/sdk/include)
target_include_directories(hexa_engine PRIVATE ${OBJ_LOADER_HOME}/Source)



# ogre
#file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg ${OGRE_CONFIG_DIR}/resources.cfg DESTINATION ${CMAKE_BINARY_DIR})