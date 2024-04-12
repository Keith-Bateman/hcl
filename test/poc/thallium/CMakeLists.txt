add_executable(thallium_server server.cpp)
target_link_libraries(thallium_server -L${THALLIUM_LIBRARY_PATH} ${THALLIUM_LIBRARIES})

add_executable(thallium_client client.cpp)
target_link_libraries(thallium_client -L${THALLIUM_LIBRARY_PATH} ${THALLIUM_LIBRARIES})