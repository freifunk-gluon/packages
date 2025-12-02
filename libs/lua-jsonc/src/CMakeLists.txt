cmake_minimum_required(VERSION 3.20)

project(lua-jsonc C)

add_library(lua-jsonc SHARED lua-jsonc.c)
set_property(TARGET lua-jsonc PROPERTY COMPILE_FLAGS "-Wall -std=c99")
target_link_libraries(lua-jsonc json-c)

install(TARGETS lua-jsonc
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lua-jsonc.h DESTINATION include)
install(DIRECTORY DESTINATION lib/lua)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../liblua-jsonc.so \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/lib/lua/jsonc.so)")
