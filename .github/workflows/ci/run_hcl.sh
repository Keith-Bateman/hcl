#!/bin/bash
export OMPI_ALLOW_RUN_AS_ROOT=1
export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
pushd build

echo "Testing unordered map test"
ctest -V -R ^unordered_map_test || exit 1
echo "Testing unordered map string test"
ctest -V -R ^unordered_map_string_test || exit 1
echo "Testing ordered map"
ctest -V -R ^map_test || exit 1
echo "Testing multimap"
ctest -V -R ^multimap_test || exit 1
echo "Testing priority queue"
ctest -V -R ^priority_queue_test || exit 1
echo "Testing queue"
ctest -V -R ^queue_test || exit 1
echo "Testing set"
ctest -V -R ^set_test || exit 1
echo "Testing concurrent unordered map test"
ctest -V -R ^hashmap_test || exit 1
echo "Testing concurrent queue test"
ctest -V -R ^concurrent_queue_test || exit 1
echo "Testing concurrent skiplist test"
ctest -V -R ^skiplist_test || exit 1

popd
