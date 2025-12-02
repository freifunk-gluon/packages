cmake_minimum_required(VERSION 3.20)

project(sse-multiplex C)

set(SSE_MULTIPLEX_SOCKET "/run/sse-multiplex.sock" CACHE STRING "Path of the socket to use")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sse-multiplex.h.in ${CMAKE_BINARY_DIR}/gen/generated/sse-multiplex.h)

set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)
include_directories(${CMAKE_BINARY_DIR}/gen)

add_executable(sse-multiplexd sse-multiplexd.c)
set_property(TARGET sse-multiplexd PROPERTY COMPILE_FLAGS "-Wall -std=c99")

add_executable(sse-multiplex sse-multiplex.c)
set_property(TARGET sse-multiplex PROPERTY COMPILE_FLAGS "-Wall -std=c99")

install(TARGETS sse-multiplexd sse-multiplex RUNTIME DESTINATION sbin)
