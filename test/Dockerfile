FROM    hdevarajan92/hcl:gcc8

ENV     HCL_DIR=/root/hcl

RUN     git clone --recursive https://github.com/scs-lab/hcl $HCL_DIR

SHELL   ["/bin/bash", "-c"]

ENV     RPC_BUILD_DIR=${HCL_DIR}/build_rpc
ENV     THALLIUM_TCP_BUILD_DIR=${HCL_DIR}/build_thallium_tcp

# Build Thallium TCP Version

ENV     BUILD_DIR=${THALLIUM_TCP_BUILD_DIR}

ENV     HCL_COMMUNICATION=THALLIUM

RUN     mkdir -p ${BUILD_DIR}

WORKDIR ${BUILD_DIR}

RUN     ${INSTALL_DIR}/bin/cmake \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
        -DCMAKE_PREFIX_PATH=${INSTALL_DIR} \
        -DCMAKE_BUILD_RPATH="${INSTALL_DIR}/lib" \
        -DCMAKE_INSTALL_RPATH="${INSTALL_DIR}/lib" \
        -DCMAKE_CXX_FLAGS="-I${INSTALL_DIR}/include -L${INSTALL_DIR}/lib" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=${INSTALL_DIR}/bin/g++ \
        -DCMAKE_C_COMPILER=${INSTALL_DIR}/bin/gcc \
        -DHCL_COMMUNICATION=${HCL_COMMUNICATION} \
        -DBUILD_TEST=ON \
        ..

RUN ${INSTALL_DIR}/bin/cmake --build . -- -j 2

# Test Thallium Version

WORKDIR ${THALLIUM_TCP_BUILD_DIR}/test

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./map_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./multimap_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./priority_queue_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./queue_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./set_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./unordered_map_test || exit 1

RUN     ${INSTALL_DIR}/bin/mpiexec -n 2 ./unordered_map_string_test || exit 1






