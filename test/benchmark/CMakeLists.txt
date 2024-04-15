# TODO: Setup Environment Variables: Machine (corona, ci) URI PROTOCOL://DEVICE/INTERFACE
# Create a script to extract ip using interface from psutil

# import psutil

# interfaces = psutil.net_if_addrs()

# for key, val in interfaces.items():
#     print(key, val[0].address)
add_executable(api_benchmark api_benchmark.cpp ${TEST_SRC})
target_link_libraries(api_benchmark ${TEST_LIBS})

set(examples map multimap priority_queue queue set unordered_map_string unordered_map)

foreach (example ${examples})
    set(test_parameters ${MPI_PROCESS_ARG} 2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/api_benchmark --ppn 2 --sp ${CMAKE_BINARY_DIR}/test/server_list "[${example}]")
    add_test(NAME ${example}_test COMMAND ${MPIEXEC_EXECUTABLE} ${test_parameters})
endforeach ()

add_test(NAME benchmark_test COMMAND ${MPIEXEC_EXECUTABLE} ${MPI_PROCESS_ARG} 2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/api_benchmark --ppn 2 --sp ${CMAKE_BINARY_DIR}/test/server_list)
