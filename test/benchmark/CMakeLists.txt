# TODO: Setup Environment Variables: Machine (corona, ci) URI PROTOCOL://DEVICE/INTERFACE
# Create a script to extract ip using interface from psutil

# import psutil

# interfaces = psutil.net_if_addrs()

# for key, val in interfaces.items():
#     print(key, val[0].address)

add_executable(api_benchmark api_benchmark.cpp ${TEST_SRC})
target_link_libraries(api_benchmark ${TEST_LIBS})

add_test(NAME unordered_map COMMAND mpirun -n 2 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/unordered_map)