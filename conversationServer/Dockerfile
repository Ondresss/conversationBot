FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    libopencv-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/k2-fsa/sherpa-onnx.git /tmp/sherpa-onnx && \
    cd /tmp/sherpa-onnx && \
    mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/sherpa-onnx

RUN ldconfig

WORKDIR /app

COPY . /app

RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

CMD ["./build/conversationServer"]
