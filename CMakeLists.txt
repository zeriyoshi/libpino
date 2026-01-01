# libpino

cmake_minimum_required(VERSION 3.25)
project(pino
  VERSION 1.0.0
  LANGUAGES C
  DESCRIPTION "Packageing Notation - PINO"
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# buildtime
include(cmake/buildtime.cmake)

include(GNUInstallDirs)

option(PINO_USE_VALGRIND "Use Valgrind if available" OFF)
option(PINO_USE_COVERAGE "Use coverage if available" OFF)
option(PINO_USE_SUPPLIMENTS "Use suppliments (debbuging feature)" OFF)
option(PINO_USE_TESTS "Use tests" OFF)

if(PINO_USE_SUPPLIMENTS)
  add_definitions(-DPINO_SUPPLIMENTS)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT EMSCRIPTEN AND NOT MSVC)
  message(STATUS "Enabled debug")

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -fno-inline -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -fno-inline -fno-omit-frame-pointer")

  find_program(VALGRIND "valgrind")
  if(PINO_USE_VALGRIND AND VALGRIND)
    message(STATUS "Found Valgrind: ${VALGRIND}")
    set(PINO_ENABLE_VALGRIND ON)
  else()
    set(PINO_ENABLE_VALGRIND OFF)
  endif()

  if(PINO_USE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    
    if(CMAKE_C_COMPILER_ID MATCHES "GNU")
      if(LCOV AND GENHTML)
        set(PINO_ENABLE_COVERAGE ON)
        add_definitions(-DPINO_COVERAGE)

        if(CMAKE_C_COMPILER_ID MATCHES "GNU")
          link_libraries(gcov)
        endif()
      else()
        message(WARNING "lcov or genhtml not found, disabling coverage")
        set(PINO_ENABLE_COVERAGE OFF)
      endif()
    else()
      message(WARNING "Coverage is supported only with GCC compilers")
      set(PINO_ENABLE_COVERAGE OFF)
    endif()
  else()
    set(PINO_ENABLE_COVERAGE OFF)
  endif()
else()
  set(PINO_ENABLE_VALGRIND OFF)
  set(PINO_ENABLE_COVERAGE OFF)
endif()

file(GLOB SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "include/*.h")

if(EMSCRIPTEN)
  include(cmake/emscripten.cmake)
endif()

add_library(pino-obj OBJECT ${SOURCES})
target_include_directories(pino-obj PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(PINO_ENABLE_COVERAGE)
  target_compile_options(pino-obj PRIVATE "--coverage")
  target_link_options(pino-obj PRIVATE "--coverage")
endif()

add_library(pino STATIC $<TARGET_OBJECTS:pino-obj>)
add_library(pino-shared SHARED $<TARGET_OBJECTS:pino-obj>)

if(PINO_ENABLE_COVERAGE)
  target_link_options(pino PRIVATE "--coverage")
endif()

install(
  FILES ${HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pino
)

install(
  TARGETS pino
  EXPORT pino-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(PINO_USE_TESTS)
  include(cmake/test.cmake)
endif()
