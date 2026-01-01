ARG PLATFORM=${BUILDPLATFORM:-linux/amd64}
ARG DEBIAN_VERSION=trixie
ARG ENABLE_VALGRIND=1
ARG ENABLE_CLANG=1

FROM --platform=${PLATFORM} debian:${DEBIAN_VERSION} AS base

RUN if test -f "/etc/debian_version"; then \
      export DEBIAN_FRONTEND="noninteractive" && \
      rm -f "/etc/apt/apt.conf.d/docker-clean" && \
      echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > "/etc/apt/apt.conf.d/keep-cache" && \
      apt-get update && \
      apt-get install -y "git" "xz-utils" "gcc" "g++" \
                         "cmake" "make" "lcov"; \
    elif test -f "/etc/alpine-release"; then \
      apk add --no-cache "git" "xz" "gcc" "g++" "cmake" \
                         "make" "lcov" "linux-headers"; \
    else \
      echo "Unsupported distribution" && exit 1; \
    fi

ARG ENABLE_VALGRIND
COPY "third_party/valgrind" "/valgrind"
RUN if test "${ENABLE_VALGRIND}" != "0"; then \
      if test -f "/etc/debian_version"; then \
        apt-get update && \
        apt-get install -y "build-essential" "automake" "autoconf" "git" "libc6-dbg"; \
      elif test -f "/etc/alpine-release"; then \
        apk add --no-cache "build-base" "automake" "autoconf" "git" "linux-headers" "perl" "bash"; \
      else \
        echo "Unsupported distribution" && exit 1; \
      fi && \
      cd "/valgrind" && \
        ./autogen.sh && \
        ./configure && \
        make -j"$(nproc)" && \
        make install && \
      cd -; \
    fi

ARG ENABLE_CLANG
RUN if test "${ENABLE_CLANG}" != "0" && test "$(uname -m)" != "i686";then \
      if test -f "/etc/debian_version"; then \
        export DEBIAN_FRONTEND="noninteractive" && \
        rm -f "/etc/apt/apt.conf.d/docker-clean" && \
        echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > "/etc/apt/apt.conf.d/keep-cache" && \
        apt-get --no-install-recommends install -y "ca-certificates" "curl" "gnupg" && \
        mkdir -p "/usr/share/keyrings" && \
        curl -sSL "https://apt.llvm.org/llvm-snapshot.gpg.key" | gpg --dearmor > "/usr/share/keyrings/llvm-archive-keyring.gpg" && \
        echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/trixie/ llvm-toolchain-trixie main" > "/etc/apt/sources.list.d/llvm.list" && \
        echo "deb [signed-by=/usr/share/keyrings/llvm-archive-keyring.gpg] http://apt.llvm.org/trixie/ llvm-toolchain-trixie-21 main" >> "/etc/apt/sources.list.d/llvm.list" && \
        apt-get update && \
        apt-get install --no-install-recommends -y \
          "clang-21" "clang-tools-21" "clang-format-21" "clang-tidy-21" \
          "libclang-rt-21-dev" "lld-21" "lldb-21" \
          "libc++-21-dev" "libc++abi-21-dev" \
          "llvm-21" "llvm-21-dev" "llvm-21-runtime" && \
        update-alternatives --install "/usr/bin/clang" clang "/usr/bin/clang-21" 100 && \
        update-alternatives --install "/usr/bin/clang++" clang++ "/usr/bin/clang++-21" 100 && \
        update-alternatives --install "/usr/bin/clang-format" clang-format "/usr/bin/clang-format-21" 100 && \
        update-alternatives --install "/usr/bin/clang-tidy" clang-tidy "/usr/bin/clang-tidy-21" 100 && \
        update-alternatives --install "/usr/bin/lldb" lldb "/usr/bin/lldb-21" 100 && \
        update-alternatives --install "/usr/bin/ld.lld" ld.lld "/usr/bin/ld.lld-21" 100 && \
        update-alternatives --install "/usr/bin/llvm-symbolizer" llvm-symbolizer "/usr/bin/llvm-symbolizer-21" 100 && \
        update-alternatives --install "/usr/bin/llvm-config" llvm-config "/usr/bin/llvm-config-21" 100; \
      elif test -f "/etc/alpine-release"; then \
        echo "Clang is not available on Alpine Linux"; \
      else \
        echo "Unsupported distribution" && exit 1; \
      fi; \
    fi

COPY . "/project"

WORKDIR "/project"

CMD [ "/project/ci.sh" ]

FROM base AS devcontainer

RUN if test -f "/etc/debian_version"; then \
      export DEBIAN_FRONTEND="noninteractive" && \
      rm -f "/etc/apt/apt.conf.d/docker-clean" && \
      echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > "/etc/apt/apt.conf.d/keep-cache" && \
      apt-get update && \
      apt-get install -y "ssh" "gdb"; \
    elif test -f "/etc/alpine-release"; then \
      apk add --no-cache "openssh" "gdb"; \
    else \
      echo "Unsupported distribution" && exit 1; \
    fi
