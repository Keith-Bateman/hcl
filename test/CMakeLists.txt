if(NOT DEFINED ENV{HCL_TEST_MACHINE})
    message(FATAL_ERROR "-- [${PROJECT_NAME}] HCL_TEST_MACHINE in env should be set for ${PROJECT_NAME} test build")
else()
    message(STATUS "[${PROJECT_NAME}] found setting machine to $ENV{HCL_TEST_MACHINE}")
endif()


if ($ENV{HCL_TEST_MACHINE} STREQUAL "corona")
    set(MPI_PROCESS_ARG -n)
else()
    set(MPI_PROCESS_ARG -np)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_LIBS ${TEST_LIBS} -lstdc++fs ${MPI_CXX_LIBRARIES} hcl)
set(TEST_LIBS_NO_MPI ${TEST_LIBS} -lstdc++fs hcl)
set(TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/catch_config.h ${CMAKE_CURRENT_SOURCE_DIR}/mpi_console_reporter.cpp ${CMAKE_CURRENT_SOURCE_DIR}/mpi_console_reporter.hpp ${CMAKE_CURRENT_SOURCE_DIR}/util.h)

add_subdirectory(benchmark)




# target_link_libraries(DistributedHashMapTest ${CMAKE_BINARY_DIR}/libhcl.so)

set(examples global_clock_test hashmap_test concurrent_queue_test skiplist_test simple_test)

add_custom_target(copy_hostfile)
add_custom_command(TARGET copy_hostfile
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/hostfile
        ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(copy_server_list)
add_custom_command(
        TARGET copy_server_list
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/server_list
        ${CMAKE_CURRENT_BINARY_DIR})

# Compile all examples
foreach (example ${examples})
    add_executable(${example} ${example}.cpp util.h)
    add_dependencies(${example} ${PROJECT_NAME})
    add_dependencies(${example} copy_hostfile)
    add_dependencies(${example} copy_server_list)
    target_include_directories(${example} SYSTEM PUBLIC ${MPI_CXX_INCLUDE_DIRS})
    set_target_properties(${example} PROPERTIES ENVIRONMENT LD_PRELOAD=${CMAKE_BINARY_DIR}/${HCL_LIBDIR}/libhcl.so:${THALLIUM_LIBRARY_LD_PRELOAD})
    target_link_libraries(${example} ${PROJECT_NAME})
    target_link_libraries(${example} ${MPI_CXX_LIBRARIES})
    set_target_properties(${example} PROPERTIES FOLDER test)
endforeach ()

message(STATUS ${CMAKE_BINARY_DIR}/libhcl.so)

# Define MPI test case template
function(mpi target mpi_procs example ranks_per_process num_requests size_of_request server_on_node debug)
    set(test_parameters ${MPI_PROCESS_ARG} ${mpi_procs} -f "${CMAKE_BINARY_DIR}/test/hostfile" "${CMAKE_BINARY_DIR}/bin/${example}" ${ranks_per_process} ${num_requests} ${size_of_request} ${server_on_node} ${debug})
    set(test_name ${target}_${example}_MPI_${mpi_procs}_${ranks_per_process}_${num_requests}_${size_of_request}_${server_on_node}_${debug})
    add_test(NAME ${test_name} COMMAND ${MPIEXEC_EXECUTABLE} ${test_parameters})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "LD_PRELOAD=${CMAKE_BINARY_DIR}/libhcl.so;LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/.spack-env/view/lib:$ENV{LD_LIBRARY_PATH};SERVER_LIST_PATH=${CMAKE_BINARY_DIR}/test/")

endfunction()

# Define MPI test case
foreach (example ${examples})
    mpi(ares 4 ${example} 2 500 1000 0 0)
endforeach ()

set(examples global_clock_test hashmap_test concurrent_queue_test skiplist_test)

foreach (example ${examples})
    mpi(ares 4 ${example} 2 500 1000 1 0)
endforeach ()

foreach (example ${examples})
    set(test_parameters ${MPI_PROCESS_ARG} 2 "${CMAKE_BINARY_DIR}/bin/${example}")
    add_test(NAME ${example} COMMAND ${MPIEXEC_EXECUTABLE} ${test_parameters})
    set_tests_properties(${example} PROPERTIES ENVIRONMENT "LD_PRELOAD=${CMAKE_BINARY_DIR}/${HCL_LIBDIR}/libhcl.so;LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/.spack-env/view/lib:$ENV{LD_LIBRARY_PATH};SERVER_LIST_PATH=${CMAKE_BINARY_DIR}/test/")
endforeach ()

add_subdirectory(poc)