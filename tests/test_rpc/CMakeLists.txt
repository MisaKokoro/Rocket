add_executable(test_rpc_client test_rpc_client.cc order.pb.cc)
add_executable(test_rpc_server test_rpc_server.cc order.pb.cc)

target_link_libraries(test_rpc_client rocket)
target_link_libraries(test_rpc_server rocket)

