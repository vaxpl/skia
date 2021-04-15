project(piex)

list(APPEND SOURCES
    ../externals/piex/src/binary_parse/cached_paged_byte_array.cc
    ../externals/piex/src/binary_parse/range_checked_byte_ptr.cc
    ../externals/piex/src/image_type_recognition/image_type_recognition_lite.cc
    ../externals/piex/src/piex.cc
    ../externals/piex/src/tiff_directory/tiff_directory.cc
    ../externals/piex/src/tiff_parser.cc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../externals/piex
)

add_definitions(
    -DBREAK_IF_DEBUGGING_AND_OUT_OF_RANGE
)

add_library(piex STATIC ${SOURCES})
