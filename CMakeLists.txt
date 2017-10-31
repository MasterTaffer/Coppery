
cmake_minimum_required (VERSION 2.6)
project (Coppery)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --std=c++14 -Wall -Wno-long-long -Wno-reorder")

find_package(GLEW REQUIRED)
find_package(Lua 5.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/src/")
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/aminc")
link_directories("${CMAKE_SOURCE_DIR}/lib")

file(GLOB_RECURSE CPPSources src/*.cpp)
list(APPEND CPPSources "src/GL/gl_compatibility_4_1.c")

file(GLOB_RECURSE HPPSources src/*.hpp)
file(GLOB_RECURSE HSources src/*.h)

add_executable(Coppery ${CPPSources})
target_compile_definitions(Coppery PRIVATE "-DCOPPERY_COMPILE_LIBRARY" "-DCOPPERY_COMPILE_FMAIN")
target_link_libraries(Coppery sfml-audio sfml-system angelscript ${OPENGL_LIBRARIES} ${LUA_LIBRARIES} ${GLFW_STATIC_LIBRARIES})


