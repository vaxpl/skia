project(jpeg-turbo)

list(APPEND SOURCES
    "../externals/libjpeg-turbo/jaricom.c"
    "../externals/libjpeg-turbo/jcapimin.c"
    "../externals/libjpeg-turbo/jcapistd.c"
    "../externals/libjpeg-turbo/jcarith.c"
    "../externals/libjpeg-turbo/jccoefct.c"
    "../externals/libjpeg-turbo/jccolor.c"
    "../externals/libjpeg-turbo/jcdctmgr.c"
    "../externals/libjpeg-turbo/jchuff.c"
    "../externals/libjpeg-turbo/jcinit.c"
    "../externals/libjpeg-turbo/jcmainct.c"
    "../externals/libjpeg-turbo/jcmarker.c"
    "../externals/libjpeg-turbo/jcmaster.c"
    "../externals/libjpeg-turbo/jcomapi.c"
    "../externals/libjpeg-turbo/jcparam.c"
    "../externals/libjpeg-turbo/jcphuff.c"
    "../externals/libjpeg-turbo/jcprepct.c"
    "../externals/libjpeg-turbo/jcsample.c"
    "../externals/libjpeg-turbo/jdapimin.c"
    "../externals/libjpeg-turbo/jdapistd.c"
    "../externals/libjpeg-turbo/jdarith.c"
    "../externals/libjpeg-turbo/jdcoefct.c"
    "../externals/libjpeg-turbo/jdcolor.c"
    "../externals/libjpeg-turbo/jddctmgr.c"
    "../externals/libjpeg-turbo/jdhuff.c"
    "../externals/libjpeg-turbo/jdinput.c"
    "../externals/libjpeg-turbo/jdmainct.c"
    "../externals/libjpeg-turbo/jdmarker.c"
    "../externals/libjpeg-turbo/jdmaster.c"
    "../externals/libjpeg-turbo/jdmerge.c"
    "../externals/libjpeg-turbo/jdphuff.c"
    "../externals/libjpeg-turbo/jdpostct.c"
    "../externals/libjpeg-turbo/jdsample.c"
    "../externals/libjpeg-turbo/jerror.c"
    "../externals/libjpeg-turbo/jfdctflt.c"
    "../externals/libjpeg-turbo/jfdctfst.c"
    "../externals/libjpeg-turbo/jfdctint.c"
    "../externals/libjpeg-turbo/jidctflt.c"
    "../externals/libjpeg-turbo/jidctfst.c"
    "../externals/libjpeg-turbo/jidctint.c"
    "../externals/libjpeg-turbo/jidctred.c"
    "../externals/libjpeg-turbo/jmemmgr.c"
    "../externals/libjpeg-turbo/jmemnobs.c"
    "../externals/libjpeg-turbo/jpeg_nbits_table.c"
    "../externals/libjpeg-turbo/jquant1.c"
    "../externals/libjpeg-turbo/jquant2.c"
    "../externals/libjpeg-turbo/jutils.c"
)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    list(APPEND SOURCES
        "../externals/libjpeg-turbo/simd/arm/arm/jsimd.c"
        "../externals/libjpeg-turbo/simd/arm/arm/jsimd_neon.S"
        "../externals/libjpeg-turbo/simd/arm/common/jcsample-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdcolor-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdmerge-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdsample-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctfst-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctint-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctred-neon.c"
    )
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    list(APPEND SOURCES
        "../externals/libjpeg-turbo/simd/arm/arm64/jsimd.c"
        "../externals/libjpeg-turbo/simd/arm/arm64/jsimd_neon.S"
        "../externals/libjpeg-turbo/simd/arm/common/jcsample-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdcolor-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdmerge-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jdsample-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctfst-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctint-neon.c"
        "../externals/libjpeg-turbo/simd/arm/common/jidctred-neon.c"
    )
else()
    list(APPEND SOURCES
        "../externals/libjpeg-turbo/jsimd_none.c"
    )
endif()

add_definitions(
    -DC_ARITH_CODING_SUPPORTED=1
    -DD_ARITH_CODING_SUPPORTED=1
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
