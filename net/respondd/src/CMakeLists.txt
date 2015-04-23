cmake_minimum_required(VERSION 2.6)

project(respondd C)

find_package(Lua51 REQUIRED)

set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
include_directories(${LUA_INCLUDE_DIR})

add_executable(respondd respondd.c)
set_property(TARGET respondd PROPERTY COMPILE_FLAGS "-Wall -std=c99")
target_link_libraries(respondd ${LUA_LIBRARIES} dl)

install(TARGETS respondd RUNTIME DESTINATION bin)
