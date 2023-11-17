file(GLOB CODER_CPP_SOURCES *.cc)
add_library(
    rocket_coder
    OBJECT
    ${CODER_CPP_SOURCES})

set(ALL_OBJECT_FILES
${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocket_coder>
PARENT_SCOPE)