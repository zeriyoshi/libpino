#!/bin/sh -eu

cd "$(dirname "${0}")"

for BUILD_TYPE in "Debug" "Release"; do
  rm -rf "build"
  cmake -B "build" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DPINO_USE_TESTS=ON .
  cmake --build "build" --parallel
  ctest --build-config "${BUILD_TYPE}" --test-dir "build" --output-on-failure
done

if test -n "$(which "clang")" -a -n "$(which "clang++")"; then
  rm -rf "build"
  cmake -B "build" \
    -DCMAKE_C_COMPILER="$(which "clang")" \
    -DCMAKE_CXX_COMPILER="$(which "clang++")" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DPINO_USE_TESTS=ON \
    .
  cmake --build "build" --parallel
  ctest --build-config "Release" --test-dir "build" --output-on-failure
  for SANITIZER_OPTIONS in "" "-fsanitize=address" "-fsanitize=undefined" "-fsanitize=memory"; do
    rm -rf "build"
    cmake -B "build" \
      -DCMAKE_C_COMPILER="$(which "clang")" \
      -DCMAKE_CXX_COMPILER="$(which "clang++")" \
      -DCMAKE_BUILD_TYPE="Debug" \
      -DCMAKE_C_FLAGS="${SANITIZER_OPTIONS}" \
      -DCMAKE_EXE_LINKER_FLAGS="${SANITIZER_OPTIONS}" \
      -DCMAKE_SHARED_LINKER_FLAGS="${SANITIZER_OPTIONS}" \
      -DCMAKE_MODULE_LINKER_FLAGS="${SANITIZER_OPTIONS}" \
      -DPINO_USE_TESTS=ON \
      .
    cmake --build "build" --parallel
    ctest --build-config "Debug" --test-dir "build" --output-on-failure
  done
fi
