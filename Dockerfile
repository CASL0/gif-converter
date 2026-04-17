# syntax=docker/dockerfile:1
FROM debian:12-slim AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ca-certificates \
    autoconf \
    automake \
    libtool \
    python3 \
    python3-setuptools \
    python3-jinja2 \
    nasm \
    yasm \
    gperf \
    bison \
    flex \
    linux-libc-dev \
    && rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/opt/vcpkg

WORKDIR /app
COPY vcpkg.json CMakePresets.json ./

SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} \
    && git -C ${VCPKG_ROOT} checkout "$(grep '"builtin-baseline"' vcpkg.json | cut -d '"' -f 4)" \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics
RUN ${VCPKG_ROOT}/vcpkg install --triplet x64-linux

COPY CMakeLists.txt ./
COPY src/ src/

# GCC 12 has a false positive -Wrestrict on quill's JsonSink.h (fixed in GCC 13)
RUN cmake --preset release -DBUILD_TESTING=OFF \
    -DCMAKE_CXX_FLAGS="-Wno-error=restrict" \
    && cmake --build build/

FROM build AS libs

RUN mkdir -p /runtime-libs \
    && ldd /app/build/src/gif-converter \
    | grep -oP '=> \K/\S+' \
    | xargs -I{} cp --parents {} /runtime-libs/

FROM gcr.io/distroless/cc-debian12:nonroot AS runtime

LABEL org.opencontainers.image.source="https://github.com/CASL0/gif-converter"
LABEL org.opencontainers.image.description="Convert video files to animated GIFs"
LABEL org.opencontainers.image.licenses="MIT"

COPY --from=libs /runtime-libs/ /
COPY --from=build /app/build/src/gif-converter /usr/local/bin/gif-converter

EXPOSE 8080
ENTRYPOINT ["gif-converter"]
