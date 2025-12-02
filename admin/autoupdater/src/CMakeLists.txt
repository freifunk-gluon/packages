cmake_minimum_required(VERSION 3.20)

project(AUTOUPDATER C)

set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS _GNU_SOURCE)

find_path(UBOX_INCLUDE_DIR NAMES libubox/uloop.h)
find_library(UBOX_LIBRARY NAMES ubox)
find_library(UCLIENT_LIBRARY NAMES uclient)

find_library(UCI_LIBRARY NAMES uci)
find_library(PLATFORMINFO_LIBRARY NAMES platforminfo)

find_package(PkgConfig REQUIRED QUIET)
pkg_check_modules(ECDSAUTIL REQUIRED ecdsautil)

include_directories(${UBOX_INCLUDE_DIR} ${ECDSAUTIL_INCLUDE_DIRS})

add_executable(autoupdater
  autoupdater.c
  hexutil.c
  manifest.c
  settings.c
  uclient.c
  util.c
  version.c
)
set_property(TARGET autoupdater PROPERTY COMPILE_FLAGS "-std=c99 -Wall")
#set_property(TARGET autoupdater PROPERTY LINK_FLAGS "")
target_link_libraries(autoupdater
    m
    ${PLATFORMINFO_LIBRARY}
    ${UCI_LIBRARY}
    ${UBOX_LIBRARY}
    ${UCLIENT_LIBRARY}
    ${ECDSAUTIL_LIBRARIES}
)

install(TARGETS autoupdater RUNTIME DESTINATION sbin)
