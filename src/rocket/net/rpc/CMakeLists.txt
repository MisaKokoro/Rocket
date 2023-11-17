file(GLOB RPC_CPP_SOURCES *.cc)
add_library(
    rocket_rpc
    OBJECT
    ${RPC_CPP_SOURCES})

set(ALL_OBJECT_FILES
${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocket_rpc>
PARENT_SCOPE)