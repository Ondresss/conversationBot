FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    meson \
    ninja-build \
    nlohmann-json3-dev \
    libcurl4-openssl-dev \
    libspdlog-dev \
    libsqlite3-dev \
    libssl-dev \
    python3 \
    python3-pip \
    ca-certificates \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    libxvidcore-dev \
    libx264-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    libopenexr-dev \
    libtbb-dev \
    libdc1394-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    pkg-config \
    tar \
    && rm -rf /var/lib/apt/lists/*

ARG BOOST_VERSION=1.92.0
ARG BOOST_UNDERSCORE=1_92_0
ARG OPENCV_VERSION=5.0.0

RUN wget -q \
        "https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_UNDERSCORE}.tar.gz" \
        -O /tmp/boost.tar.gz && \
    mkdir -p /tmp/boost && \
    tar -xzf /tmp/boost.tar.gz \
        --strip-components=1 \
        -C /tmp/boost && \
    cd /tmp/boost && \
    ./bootstrap.sh --prefix=/opt/boost && \
    ./b2 -j"$(nproc)" install && \
    rm -rf /tmp/boost /tmp/boost.tar.gz

RUN echo "/opt/boost/lib" > /etc/ld.so.conf.d/boost.conf && ldconfig

ENV BOOST_ROOT=/opt/boost
ENV CMAKE_PREFIX_PATH=/opt/boost
ENV CPLUS_INCLUDE_PATH=/opt/boost/include
ENV LIBRARY_PATH=/opt/boost/lib
ENV LD_LIBRARY_PATH=/opt/boost/lib

RUN git clone \
        --branch "${OPENCV_VERSION}" \
        --depth 1 \
        https://github.com/opencv/opencv.git \
        /tmp/opencv && \
    cmake -S /tmp/opencv -B /tmp/opencv/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DBUILD_TESTS=OFF \
        -DBUILD_PERF_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_opencv_python3=OFF \
        -DWITH_GTK=ON \
        -DWITH_TBB=ON && \
    cmake --build /tmp/opencv/build \
        --parallel "$(nproc)" && \
    cmake --install /tmp/opencv/build && \
    rm -rf /tmp/opencv && \
    ldconfig

RUN grep -E 'BOOST_LIB_VERSION|BOOST_VERSION' \
        /opt/boost/include/boost/version.hpp && \
    find /opt/boost/include/boost/process \
        -name pipe.hpp -print && \
    find /usr/local/include \
        -path '*/opencv2/photo/ccm.hpp' -print && \
    find /usr/local \
        -name OpenCVConfig.cmake -print

RUN git clone https://github.com/k2-fsa/sherpa-onnx.git /tmp/sherpa-onnx && \
    cmake -S /tmp/sherpa-onnx \
        -B /tmp/sherpa-onnx/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DSHERPA_ONNX_ENABLE_PYTHON=OFF \
        -DSHERPA_ONNX_ENABLE_BINARY=ON \
        -DBOOST_ROOT=/opt/boost \
        -DCMAKE_PREFIX_PATH=/usr/local:/opt/boost && \
    cmake --build /tmp/sherpa-onnx/build \
        --parallel "$(nproc)" && \
    cmake --install /tmp/sherpa-onnx/build && \
    rm -rf /tmp/sherpa-onnx && \
    ldconfig

RUN git clone https://github.com/pistacheio/pistache.git /tmp/pistache && \
    cd /tmp/pistache && \
    meson setup build \
        --prefix=/usr/local \
        --libdir=lib \
        -DPISTACHE_BUILD_TESTS=false \
        -DPISTACHE_BUILD_EXAMPLES=false && \
    meson compile -C build && \
    meson install -C build && \
    rm -rf /tmp/pistache

RUN ldconfig

WORKDIR /app

COPY . /app

RUN find /usr/local \
    -name OpenCVConfig.cmake \
    -print

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBOOST_ROOT=/opt/boost \
    -DBoost_ROOT=/opt/boost \
    -DBoost_NO_SYSTEM_PATHS=ON \
    -DCMAKE_PREFIX_PATH=/usr/local:/opt/boost \
    -DOpenCV_DIR=/usr/local/lib/cmake/opencv4 && \
cmake --build build \
    --parallel "$(nproc)"

RUN wget -q https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_linux_x86_64.tar.gz -O /tmp/piper.tar.gz && \
    tar -xzf /tmp/piper.tar.gz -C /opt/ && \
    rm /tmp/piper.tar.gz

ENV PATH="/opt/piper:$PATH"

CMD ["./build/conversationServer"]
