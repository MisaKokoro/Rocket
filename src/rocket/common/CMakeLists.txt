file(GLOB COMMON_CPP_SOURCES *.cc)
add_library(
    rocket_common
    OBJECT
    ${COMMON_CPP_SOURCES})

set(ALL_OBJECT_FILES
${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocket_common>
PARENT_SCOPE)