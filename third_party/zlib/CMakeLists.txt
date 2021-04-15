project(zlib)

list(APPEND SOURCES
    ../externals/zlib/adler32.c
    ../externals/zlib/compress.c
    ../externals/zlib/cpu_features.c
    ../externals/zlib/crc32.c
    ../externals/zlib/deflate.c
    ../externals/zlib/gzclose.c
    ../externals/zlib/gzlib.c
    ../externals/zlib/gzread.c
    ../externals/zlib/gzwrite.c
    ../externals/zlib/infback.c
    ../externals/zlib/inffast.c
    ../externals/zlib/inftrees.c
    ../externals/zlib/trees.c
    ../externals/zlib/uncompr.c
    ../externals/zlib/zutil.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../externals/zlib
)

add_library(z STATIC ${SOURCES})
