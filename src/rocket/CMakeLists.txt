add_subdirectory(common)
add_subdirectory(net)

add_library(rocket STATIC ${ALL_OBJECT_FILES})
set(ROCKET_LIBS
    rocket_common
    rocket_net
    rocket_coder
    rocket_rpc
    rocket_tcp)

set(ROCKET_THIRD_PARTY_LIBS
    protobuf
    tinyxml)

target_link_libraries(
    rocket
    ${ROCKET_LIBS}
    ${ROCKET_THIRD_PARTY_LIBS}
)

target_include_directories(
    rocket
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include   
)