FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    ninja-build \
    clang-format \
    autoconf \
    automake \
    libtool \
    nasm \
    && rm -rf /var/lib/apt/lists/*

COPY vcpkg.json /tmp/vcpkg.json

ENV VCPKG_ROOT=/usr/local/vcpkg
RUN cd ${VCPKG_ROOT} \
    && git fetch origin $(grep '"builtin-baseline"' /tmp/vcpkg.json | cut -d '"' -f 4) \
    && git -c advice.detachedHead=false checkout FETCH_HEAD \
    && ./bootstrap-vcpkg.sh -disableMetrics \
    && chown -R vscode:vscode ${VCPKG_ROOT} \
    && rm /tmp/vcpkg.json
