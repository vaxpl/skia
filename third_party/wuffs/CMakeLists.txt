project(wuffs)

list(APPEND SOURCES
    ../externals/wuffs/release/c/wuffs-v0.3.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../externals/wuffs
)

add_definitions(
    -DBREAK_IF_DEBUGGING_AND_OUT_OF_RANGE
)

add_library(wuffs STATIC ${SOURCES})

