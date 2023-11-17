add_subdirectory(coder)
add_subdirectory(tcp)
add_subdirectory(rpc)

file(GLOB NET_CPP_SOURCES *.cc)
add_library(
    rocket_net
    OBJECT
    ${NET_CPP_SOURCES}
)

set(ALL_OBJECT_FILES
${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocket_net>
PARENT_SCOPE)