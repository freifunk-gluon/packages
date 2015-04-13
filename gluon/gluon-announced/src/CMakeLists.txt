cmake_minimum_required(VERSION 2.6)

INCLUDE(FindPkgConfig)

pkg_check_modules(LUA REQUIRED lua>=5.1)

PROJECT(gluon-announced C)

ADD_DEFINITIONS(-Os -Wall --std=gnu99 -Wmissing-declarations)

SET(SOURCES gluon-announced.c)

ADD_DEFINITIONS(${LUA_CFLAGS})
ADD_EXECUTABLE(gluon-announced ${SOURCES})
TARGET_LINK_LIBRARIES(gluon-announced ${LUA_LIBRARIES} dl m)
INSTALL(TARGETS gluon-announced
	RUNTIME DESTINATION bin
)
