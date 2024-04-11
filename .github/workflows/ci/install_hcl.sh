#!/bin/bash

INSTALL_DIR=/root/install
VIEW_DIR=$HOME/install/.spack-env/view
SPACK_DIR=/root/spack

mkdir build
pushd build

CXXFLAGS="-I${VIEW_DIR}/include"
LDFLAGS="-L${VIEW_DIR}/lib"
CMAKE_C_COMPILER=`which gcc`
CMAKE_CXX_COMPILER=`which g++`

echo ${VIEW_DIR}

cmake \
    -DCMAKE_INSTALL_PREFIX=${VIEW_DIR} \
    -DCMAKE_PREFIX_PATH=${VIEW_DIR} \
    -DCMAKE_BUILD_RPATH="${VIEW_DIR}/lib" \
    -DCMAKE_INSTALL_RPATH="${VIEW_DIR}/lib" \
    -DCMAKE_CXX_FLAGS="-I${VIEW_DIR}/include -L${VIEW_DIR}/lib" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} \
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
    -DHCL_COMMUNICATION=${HCL_COMMUNICATION} \
    -DHCL_COMMUNICATION_PROTOCOL=${HCL_COMMUNICATION_PROTOCOL} \
    -DHCL_ENABLE_TESTING=ON \
    ../

cmake --build . -- -j 2 VERBOSE=1 || exit 1

popd
