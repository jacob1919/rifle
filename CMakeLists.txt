cmake_minimum_required(VERSION 3.20)

project(rifle
    VERSION 0.1.0
    DESCRIPTION "Hands-on GGUF inference engine"
    LANGUAGES CXX
)

add_library(rifle_core STATIC
    src/core/gguf_parser.cpp
)

target_compile_features(rifle_core PUBLIC cxx_std_20)

target_include_directories(rifle_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(rifle_core PRIVATE /W4 /permissive- /Zc:__cplusplus)
    target_compile_definitions(rifle_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    target_compile_options(rifle_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(rifle-gguf-info
    tools/gguf_info.cpp
)

target_link_libraries(rifle-gguf-info PRIVATE rifle_core)

