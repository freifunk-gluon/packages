cmake_minimum_required(VERSION 2.6)
project(LIBPLATFORMINFO C)

set(LIBDIR "lib${LIB_SUFFIX}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(TARGET "" CACHE STRING "OpenWrt target")
set(SUBTARGET "" CACHE STRING "OpenWrt subtarget")

add_definitions(-DTARGET=${TARGET})

if(SUBTARGET)
  set(FULL_TARGET "${TARGET}-${SUBTARGET}")

  add_definitions(-DSUBTARGET=${SUBTARGET} -DTARGET_${TARGET}_${SUBTARGET})
else(SUBTARGET)
  set(FULL_TARGET "${TARGET}")

  add_definitions(-DTARGET_${TARGET})
endif(SUBTARGET)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/targets/${FULL_TARGET}.c")
  set(TARGET_SOURCE "targets/${FULL_TARGET}.c")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/targets/${TARGET}.c")
  set(TARGET_SOURCE "targets/${TARGET}.c")
else()
  set(TARGET_SOURCE "targets/default.c")
endif()


add_library(platforminfo SHARED
  common.c
  ${TARGET_SOURCE}
)
set_target_properties(platforminfo PROPERTIES
  COMPILE_FLAGS "-Wall -std=c99 -D_GNU_SOURCE"
)
install(TARGETS platforminfo
  ARCHIVE DESTINATION ${LIBDIR}
  LIBRARY DESTINATION ${LIBDIR}
)


install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/libplatforminfo.h DESTINATION include)
