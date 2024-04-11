#!/bin/bash

INSTALL_DIR=/root/install
SPACK_DIR=/root/spack

mkdir build
pushd build

CXXFLAGS="-I${INSTALL_DIR}/include"
LDFLAGS="-L${INSTALL_DIR}/lib"
CMAKE_C_COMPILER=`which gcc`
CMAKE_CXX_COMPILER=`which g++`

echo ${INSTALL_DIR}

cmake \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
    -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
    -DCMAKE_BUILD_RPATH="${INSTALL_DIR}/lib" \
    -DCMAKE_INSTALL_RPATH="${INSTALL_DIR}/lib" \
    -DCMAKE_CXX_FLAGS="-I${INSTALL_DIR}/include -L${INSTALL_DIR}/lib" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} \
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} \
    -DHCL_COMMUNICATION=${HCL_COMMUNICATION} \
    -DBUILD_TEST=ON \
    ..

cmake --build . -- -j 2 VERBOSE=1 || exit 1

popd
