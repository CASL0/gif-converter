FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    ninja-build \
    clang-format \
    autoconf \
    automake \
    libtool \
    nasm \
    doxygen \
    graphviz \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

ARG DROGON_VERSION=v1.9.12
RUN git clone --depth 1 --branch ${DROGON_VERSION} --recursive \
        https://github.com/drogonframework/drogon.git /tmp/drogon \
    && cmake -S /tmp/drogon -B /tmp/drogon/build -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_ORM=OFF \
    && cmake --build /tmp/drogon/build --target drogon_ctl \
    && install -m 755 /tmp/drogon/build/drogon_ctl/drogon_ctl /usr/local/bin/ \
    && rm -rf /tmp/drogon

COPY vcpkg.json /tmp/vcpkg.json

ENV VCPKG_ROOT=/usr/local/vcpkg
RUN cd ${VCPKG_ROOT} \
    && git fetch origin $(grep '"builtin-baseline"' /tmp/vcpkg.json | cut -d '"' -f 4) \
    && git -c advice.detachedHead=false checkout FETCH_HEAD \
    && ./bootstrap-vcpkg.sh -disableMetrics \
    && chown -R vscode:vscode ${VCPKG_ROOT} \
    && rm /tmp/vcpkg.json
