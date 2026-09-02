cmake_minimum_required(VERSION 3.16)
project(rnllama VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  # O3 is a clang-only define; MSVC's Release config already selects /O2.
  # _USE_MATH_DEFINES makes MSVC's <cmath> define M_PI & friends (used by
  # codec sources), which it otherwise leaves undefined.
  add_definitions(
    /DNDEBUG
    /DLM_GGML_USE_CPU
    /DLM_GGML_USE_CPU_REPACK
    /D_USE_MATH_DEFINES
  )
else ()
  add_definitions(
    -DNDEBUG
    -DO3
    -DLM_GGML_USE_CPU
    -DLM_GGML_USE_CPU_REPACK
  )
endif ()

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp)

# Collect source files (same set as ios/CMakeLists.txt minus Metal/BLAS,
# plus the x86 SIMD kernels which Windows uses).
file(GLOB MODEL_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/models/*.cpp)
file(GLOB GGML_CPU_C_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/ggml-cpu/*.c)
file(GLOB GGML_CPU_CPP_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/ggml-cpu/*.cpp)
file(GLOB GGML_CPU_AMX_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/ggml-cpu/amx/*.cpp)
file(GLOB GGML_CPU_ARCH_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/ggml-cpu/arch/x86/*.c ${SOURCE_DIR}/ggml-cpu/arch/x86/*.cpp)
file(GLOB LLAMA_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/llama*.cpp)
file(GLOB MTMD_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/tools/mtmd/*.cpp)
file(GLOB MTMD_MODEL_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/tools/mtmd/models/*.cpp)
file(GLOB COMMON_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/common/*.cpp)
file(GLOB JINJA_FILES CONFIGURE_DEPENDS ${SOURCE_DIR}/common/jinja/*.cpp)
file(GLOB CODEC_FILES CONFIGURE_DEPENDS
    ${SOURCE_DIR}/codec/src/*.cpp
    ${SOURCE_DIR}/codec/src/batch/*.cpp
    ${SOURCE_DIR}/codec/src/models/*.cpp
    ${SOURCE_DIR}/codec/src/ops/*.cpp
    ${SOURCE_DIR}/codec/src/runtime/*.cpp
    ${SOURCE_DIR}/codec/src/lm/*.cpp
    ${SOURCE_DIR}/codec/common/*.cpp
)
list(REMOVE_ITEM CODEC_FILES
    ${SOURCE_DIR}/codec/common/tts_runner.cpp
    ${SOURCE_DIR}/codec/common/tts_runner_flow.cpp
)

set(PUBLIC_HEADERS
    ${SOURCE_DIR}/rn-llama.h
    ${SOURCE_DIR}/rn-completion.h
    ${SOURCE_DIR}/rn-slot.h
    ${SOURCE_DIR}/rn-slot-manager.h
    ${SOURCE_DIR}/rn-tts.h
    ${SOURCE_DIR}/llama.h
    ${SOURCE_DIR}/llama-impl.h
    ${SOURCE_DIR}/common/speculative.h
    ${SOURCE_DIR}/ggml.h
)

# Prebuilt llama.cpp core: a static library that the RNW module (.dll) links.
add_library(rnllama STATIC
    ${SOURCE_DIR}/ggml.c
    ${SOURCE_DIR}/ggml-alloc.c
    ${SOURCE_DIR}/ggml-backend.cpp
    ${SOURCE_DIR}/ggml-backend-dl.cpp
    ${SOURCE_DIR}/ggml-backend-meta.cpp
    ${SOURCE_DIR}/ggml-backend-reg.cpp
    ${SOURCE_DIR}/ggml-opt.cpp
    ${SOURCE_DIR}/ggml-threading.cpp
    ${SOURCE_DIR}/ggml-quants.c
    ${SOURCE_DIR}/gguf.cpp

    ${GGML_CPU_C_FILES}
    ${GGML_CPU_CPP_FILES}
    ${GGML_CPU_AMX_FILES}
    ${GGML_CPU_ARCH_FILES}

    ${LLAMA_FILES}
    ${SOURCE_DIR}/unicode-data.cpp
    ${SOURCE_DIR}/unicode.cpp

    ${COMMON_FILES}

    ${SOURCE_DIR}/anyascii.c
    ${SOURCE_DIR}/hash/hash.cpp
    ${SOURCE_DIR}/hash/sha256/sha256.c

    ${JINJA_FILES}

    ${CODEC_FILES}

    ${SOURCE_DIR}/common/chat.h
    ${SOURCE_DIR}/common/common.h
    ${SOURCE_DIR}/common/speculative.h

    ${MTMD_MODEL_FILES}
    ${MTMD_FILES}

    ${SOURCE_DIR}/rn-llama.cpp
    ${SOURCE_DIR}/rn-completion.cpp
    ${SOURCE_DIR}/rn-slot.cpp
    ${SOURCE_DIR}/rn-slot-manager.cpp
    ${SOURCE_DIR}/rn-tts.cpp

    ${MODEL_FILES}
)

target_include_directories(rnllama
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/common>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/common/jinja>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/hash>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/ggml-cpu>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/tools/mtmd>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/codec/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../cpp/codec/common>
)

# Public headers so a bare .lib can be consumed by the RNW module build.
set_target_properties(rnllama PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
    CXX_STANDARD 17
)