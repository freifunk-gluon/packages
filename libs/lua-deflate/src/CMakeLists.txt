cmake_minimum_required(VERSION 2.6)

project(lua-deflate C)

set(LUA_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/lua" CACHE PATH "Lua module target path")

find_package(Lua51 REQUIRED)

set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
include_directories(${LUA_INCLUDE_DIR})

add_library(deflate MODULE deflate.c)
set_property(TARGET deflate PROPERTY COMPILE_FLAGS "-Wall -std=c99 -fno-strict-aliasing")
set_property(TARGET deflate PROPERTY PREFIX "")
target_link_libraries(deflate ${LUA_LIBRARIES})

install(TARGETS deflate DESTINATION ${LUA_MODULE_PATH})
