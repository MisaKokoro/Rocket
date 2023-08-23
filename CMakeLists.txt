cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wno-deprecated -Wno-unused-but-set-variable -Wno-format-security")

# 设置源文件的路径
set(PATH_ROCKET rocket)
set(PATH_COMM ${PATH_ROCKET}/common)
set(PATH_NET ${PATH_ROCKET}/net)
set(PATH_TESTCASES testcases)
set(PATH_TCP ${PATH_NET}/tcp)
set(PAHT_CODER ${PATH_NET}/coder)
set(PATH_RPC ${PATH_NET}/rpc)

# 设置输出文件的路径
set(PATH_BIN ${CMAKE_BINARY_DIR}/bin)
set(PATH_LIB ${CMAKE_BINARY_DIR}/lib)
set(PATH_OBJ ${CMAKE_BINARY_DIR}/obj)

# 设置安装路径
set(PATH_INSTALL_INC_ROOT /usr/include)
set(PATH_INSTALL_INC_COMM ${PATH_INSTALL_INC_ROOT}/${PATH_COMM})
set(PATH_INSTALL_INC_NET ${PATH_INSTALL_INC_ROOT}/${PATH_NET})
set(PATH_INSTALL_LIB_ROOT /usr/lib)

# 添加头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PATH_ROCKET}
    ${PATH_COMM}
    ${PATH_NET}
    ${PATH_TCP}
    ${PAHT_CODER}
    ${PATH_RPC}
)

# 添加链接库搜索路径
link_directories(/usr/local/lib /usr/lib)

# 设置源文件列表
file(GLOB COMM_SRC ${PATH_COMM}/*.cc)
file(GLOB NET_SRC ${PATH_NET}/*.cc)
file(GLOB TCP_SRC ${PATH_TCP}/*.cc)
file(GLOB CODER_SRC ${PAHT_CODER}/*.cc)
file(GLOB TEST_LOG_SRC ${PATH_TESTCASES}/test_log.cc)
file(GLOB TEST_EVENTLOOP_SRC ${PATH_TESTCASES}/test_eventloop.cc)
file(GLOB TEST_TCP_SRC ${PATH_TESTCASES}/test_tcp.cc)
file(GLOB TEST_CLIENT_SRC ${PATH_TESTCASES}/test_client.cc)
file(GLOB RPC_SRC ${PATH_RPC}/*.cc)






# 生成目标可执行文件
add_executable(test_log ${TEST_LOG_SRC})
add_executable(test_eventloop ${TEST_EVENTLOOP_SRC})
add_executable(test_tcp ${TEST_TCP_SRC})
add_executable(test_client ${TEST_CLIENT_SRC})


# 链接库文件
target_link_libraries(test_log rocket protobuf tinyxml dl pthread)
target_link_libraries(test_eventloop rocket protobuf tinyxml dl pthread)
target_link_libraries(test_tcp rocket protobuf tinyxml dl pthread)
target_link_libraries(test_client rocket protobuf tinyxml dl pthread)


# 生成库文件
add_library(rocket STATIC ${RPC_SRC} ${COMM_SRC} ${NET_SRC} ${TCP_SRC} ${CODER_SRC})

# message("${PATH_RPC}")

# 设置库文件的输出路径
set_target_properties(rocket PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PATH_LIB}
)

# 安装头文件和库文件
install(DIRECTORY ${PATH_COMM} DESTINATION ${PATH_INSTALL_INC_ROOT})
install(DIRECTORY ${PATH_NET} DESTINATION ${PATH_INSTALL_INC_ROOT})
install(TARGETS rocket DESTINATION ${PATH_INSTALL_LIB_ROOT})

# 清理生成的文件
add_custom_target(clean-all
    COMMAND ${CMAKE_BUILD_TOOL} clean
    COMMAND rm -rf ${PATH_LIB} ${PATH_OBJ}
)