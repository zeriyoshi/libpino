# libpino test

include(CTest)
include(FetchContent)
include(CheckIncludeFile)

enable_testing()

FetchContent_Declare(unity SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/unity)
FetchContent_MakeAvailable(unity)

set(PINO_TEST_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
add_definitions(-DPINO_TEST_ASSETS_DIR="${PINO_TEST_ASSETS_DIR}")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

set(PINO_TEST_LINK_LIBRARIES pino unity)

if(PINO_ENABLE_COVERAGE)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage)
  add_custom_target(coverage
    COMMAND ${LCOV} --initial --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage/base.info
    COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
    COMMAND ${LCOV} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage/test.info
    COMMAND ${LCOV} --add-tracefile ${CMAKE_BINARY_DIR}/coverage/base.info --add-tracefile ${CMAKE_BINARY_DIR}/coverage/test.info --output-file ${CMAKE_BINARY_DIR}/coverage/total.info
    COMMAND ${LCOV} --remove ${CMAKE_BINARY_DIR}/coverage/total.info '/usr/*' --output-file ${CMAKE_BINARY_DIR}/coverage/filtered.info --ignore-errors negative
    COMMAND ${GENHTML} --demangle-cpp --legend --title "pino Coverage Report" --output-directory ${CMAKE_BINARY_DIR}/coverage/html ${CMAKE_BINARY_DIR}/coverage/filtered.info
    COMMENT "Generating coverage report with lcov/gcov"
  )

  message(STATUS "Coverage report generation target 'coverage' is now available")
endif()

file(GLOB TEST_SOURCES "tests/test_*.c")

foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
  set(TEST_NAME "pino_${TEST_NAME}")

  add_executable(${TEST_NAME} ${TEST_SOURCE})

  target_link_libraries(${TEST_NAME} PRIVATE pino unity)

  target_include_directories(${TEST_NAME} PRIVATE ${unity_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

  if(PINO_USE_SIMD)
    if(EMSCRIPTEN)
      target_compile_options(${TEST_NAME} PRIVATE -msimd128)
      target_compile_definitions(${TEST_NAME} PRIVATE PINO_USE_SIMD=1 PINO_SIMD_WASM=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)")
      target_compile_definitions(${TEST_NAME} PRIVATE PINO_USE_SIMD=1 PINO_SIMD_NEON=1)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)")
      if(MSVC)
        target_compile_options(${TEST_NAME} PRIVATE /arch:AVX2)
      else()
        target_compile_options(${TEST_NAME} PRIVATE -mavx2)
      endif()
      target_compile_definitions(${TEST_NAME} PRIVATE PINO_USE_SIMD=1 PINO_SIMD_AVX2=1)
    else()
      target_compile_definitions(${TEST_NAME} PRIVATE PINO_USE_SIMD=0)
    endif()
  else()
    target_compile_definitions(${TEST_NAME} PRIVATE PINO_USE_SIMD=0)
  endif()

  set_target_properties(${TEST_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
  )

  if(PINO_ENABLE_COVERAGE)
    target_compile_options(${TEST_NAME} PRIVATE "--coverage")
    target_link_options(${TEST_NAME} PRIVATE "--coverage")
  endif()

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

  if(PINO_ENABLE_VALGRIND)
    add_test(
      NAME ${TEST_NAME}_valgrind
      COMMAND ${VALGRIND} 
        "--tool=memcheck"
        "--leak-check=full"
        "--show-leak-kinds=all"
        "--track-origins=yes"
        "--error-exitcode=1"
      $<TARGET_FILE:${TEST_NAME}>)
  endif()
endforeach()
