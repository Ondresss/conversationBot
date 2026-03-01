# Conversation Bot (Semestral project)

### Models used
1. https://github.com/k2-fsa/sherpa-onnx/blob/master/cxx-api-examples/streaming-zipformer-cxx-api.cc
2. https://k2-fsa.github.io/sherpa/onnx/sense-voice/index.html

### Install models
1. wget https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-zipformer-en-2023-06-26.tar.bz2
2. tar xvf sherpa-onnx-zipformer-en-2023-06-26.tar.bz2
3. wget https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-streaming-zipformer-en-2023-06-26.tar.bz2
4. tar xvf sherpa-onnx-streaming-zipformer-en-2023-06-26.tar.bz2

### Install Sherpa-ONNX
1. git clone https://github.com/k2-fsa/sherpa-onnx
2. cd sherpa-onnx
3. mkdir build
4. cd build

5. cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DSHERPA_ONNX_ENABLE_PYTHON=OFF \
  -DSHERPA_ONNX_ENABLE_BINARY=ON \
  ..

6. make -j$(nproc)

### Install RT-audio (Source)
1. git clone https://github.com/thestk/rtaudio.git
2. cd rtaudio
3. mkdir build && cd build
4. cmake ..
5. make
6. sudo make install

### Install RT-audio (Package)
1. sudo apt update
2. sudo apt install librtaudio-dev

