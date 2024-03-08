make clean
rm -rf cmake
mkdir -p cmake/build
pushd cmake/build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR -DCMAKE_BUILD_TYPE=Debug ../..
make -j 4
