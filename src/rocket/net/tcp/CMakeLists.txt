file(GLOB TCP_CPP_SOURCES *.cc)
add_library(
    rocket_tcp
    OBJECT
    ${TCP_CPP_SOURCES})

set(ALL_OBJECT_FILES
${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocket_tcp>
PARENT_SCOPE)