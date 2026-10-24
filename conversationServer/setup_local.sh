#!/bin/bash

set -e

echo "Installing packages............"
sudo apt-get update && sudo apt-get install -y \
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
    tar
echo "Done installing packages...."

BOOST_VERSION="1.92.0"
BOOST_UNDERSCORE="1_92_0"
OPENCV_VERSION="5.0.0"

echo "=== 2. Compiling boost library ==="
wget -q "https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_UNDERSCORE}.tar.gz" -O /tmp/boost.tar.gz
mkdir -p /tmp/boost
tar -xzf /tmp/boost.tar.gz --strip-components=1 -C /tmp/boost
cd /tmp/boost
./bootstrap.sh --prefix=/opt/boost
sudo ./b2 -j"$(nproc)" install
cd - > /dev/null
rm -rf /tmp/boost /tmp/boost.tar.gz

echo "/opt/boost/lib" | sudo tee /etc/ld.so.conf.d/boost.conf > /dev/null
sudo ldconfig

echo "Done compiling boost library...."

export BOOST_ROOT=/opt/boost
export CMAKE_PREFIX_PATH=/opt/boost
export CPLUS_INCLUDE_PATH=/opt/boost/include
export LIBRARY_PATH=/opt/boost/lib
export LD_LIBRARY_PATH=/opt/boost/lib

echo "=== 3. Compiling opencv ${OPENCV_VERSION} ==="
git clone --branch "${OPENCV_VERSION}" --depth 1 https://github.com/opencv/opencv.git /tmp/opencv
cmake -S /tmp/opencv -B /tmp/opencv/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DBUILD_TESTS=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_opencv_python3=OFF \
    -DWITH_GTK=ON \
    -DWITH_TBB=ON
cmake --build /tmp/opencv/build --parallel "$(nproc)"
sudo cmake --install /tmp/opencv/build
rm -rf /tmp/opencv
sudo ldconfig

echo "Done compiling opencv...."

echo "=== 4. Compiling Sherpa-ONNX ==="
git clone https://github.com/k2-fsa/sherpa-onnx.git /tmp/sherpa-onnx
cmake -S /tmp/sherpa-onnx -B /tmp/sherpa-onnx/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DSHERPA_ONNX_ENABLE_PYTHON=OFF \
    -DSHERPA_ONNX_ENABLE_BINARY=ON \
    -DBOOST_ROOT=/opt/boost \
    -DCMAKE_PREFIX_PATH=/usr/local:/opt/boost
cmake --build /tmp/sherpa-onnx/build --parallel "$(nproc)"
sudo cmake --install /tmp/sherpa-onnx/build
rm -rf /tmp/sherpa-onnx
sudo ldconfig
echo "Done compiling sherpa-onnx......"

echo "=== 5. Compiling Pistache ==="
git clone https://github.com/pistacheio/pistache.git /tmp/pistache
cd /tmp/pistache
meson setup build \
    --prefix=/usr/local \
    --libdir=lib \
    -DPISTACHE_BUILD_TESTS=false \
    -DPISTACHE_BUILD_EXAMPLES=false
meson compile -C build
sudo meson install -C build
cd - > /dev/null
rm -rf /tmp/pistache
sudo ldconfig

echo "Done compiling pistache...."

echo "=== 6. Downloading Piper TTS ==="
sudo wget -q https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_linux_x86_64.tar.gz -O /tmp/piper.tar.gz
sudo tar -xzf /tmp/piper.tar.gz -C /opt/
rm /tmp/piper.tar.gz

echo "=== 7. Bulding project (conversationServer) ==="
mkdir -p build
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBOOST_ROOT=/opt/boost \
    -DBoost_ROOT=/opt/boost \
    -DBoost_NO_SYSTEM_PATHS=ON \
    -DCMAKE_PREFIX_PATH=/usr/local:/opt/boost \
    -DOpenCV_DIR=/usr/local/lib/cmake/opencv4

cmake --build build --parallel "$(nproc)"

echo "PROJECT BUILT!!"
