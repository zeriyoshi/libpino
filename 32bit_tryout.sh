#!/bin/sh -eu

cd "$(dirname "${0}")"
docker build \
    -t "pino-32bit-tryout" \
    --build-arg "PLATFORM=linux/i386" \
    --build-arg "IMAGE=alpine:latest" \
    --build-arg "ENABLE_VALGRIND=0" \
    .
docker run --rm --volume "$(pwd):/pino" --entrypoint "/bin/bash" -i "pino-big-endian-tryout" -c \
    "cd \"/pino\" && rm -rf \"build\" && mkdir \"build\" && cd \"build\" && cmake .. -DCMAKE_BUILD_TYPE=\"Release\" \
    -DPINO_USE_TESTS=ON -DPINO_USE_SUPPLIMENTS=ON && cmake --build . --parallel && ctest -C \"Release\" --output-on-failure"
