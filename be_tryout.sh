#!/bin/sh -eu

cd "$(dirname "${0}")"
docker build \
    -t "pino-big-endian-tryout" \
    --build-arg "PLATFORM=linux/s390x" \
    --build-arg "IMAGE=alpine:latest" \
    --build-arg "ENABLE_VALGRIND=0" \
    .
docker run --rm --volume "$(pwd):/pino" --entrypoint "/bin/bash" -i "pino-big-endian-tryout" -c \
    "cd \"/pino\" && rm -rf \"build\" && mkdir \"build\" && cd \"build\" && cmake .. -DCMAKE_BUILD_TYPE=\"Release\" \
    -DPINO_USE_TESTS=ON && cmake --build . --parallel && ctest -C \"Release\" --output-on-failure"
