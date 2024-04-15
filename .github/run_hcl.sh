#!/bin/bash

export OMPI_ALLOW_RUN_AS_ROOT=1
export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
export UCX_LOG_LEVEL=info
pushd build


ctest -VV -R benchmark_test || exit 1

if [ "${HCL_COMMUNICATION}" = "THALLIUM" ]; then
    ctest -V -R ^hashmap_test || exit 1
    echo "Testing concurrent queue test"
    ctest -V -R ^concurrent_queue_test || exit 1
    echo "Testing concurrent skiplist test"
    ctest -V -R ^skiplist_test || exit 1
fi
popd
