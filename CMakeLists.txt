cmake_minimum_required(VERSION 3.1)

project(skia
    VERSION 92
    LANGUAGES C CXX
)

# Module options
option(SKIA_HAVE_CODEC          "Enable codec module"            ON)
option(SKIA_HAVE_CODEC_JPEG     "Enable codec/jpeg supports"     ON)
option(SKIA_HAVE_CODEC_PNG      "Enable codec/png supports"      ON)
option(SKIA_HAVE_CODEC_WEBP     "Enable codec/webp supports"     OFF)
option(SKIA_HAVE_CODEC_WUFFS    "Enable codec/wuffs supports"    OFF)
option(SKIA_HAVE_EFFECTS        "Enable effects module"          OFF)

# Compiler options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(third_party)
add_subdirectory(src)
