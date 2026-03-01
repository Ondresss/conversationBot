git clone https://github.com/k2-fsa/sherpa-onnx
cd sherpa-onnx
mkdir build
cd build

cmake \
-DCMAKE_BUILD_TYPE=Release \
-DSHERPA_ONNX_ENABLE_PYTHON=OFF \
-DSHERPA_ONNX_ENABLE_BINARY=ON \
..

make -j$(nproc)

wget https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-zipformer-en-2023-06-26.tar.bz2
tar xvf sherpa-onnx-zipformer-en-2023-06-26.tar.bz2