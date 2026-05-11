cmake_minimum_required(VERSION 3.20)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT DEFINED AEROGEL_ARCH)
    set(AEROGEL_ARCH "x86_64" CACHE STRING "Target architecture: x86 or x86_64")
endif()

if(AEROGEL_ARCH STREQUAL "x86")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
elseif(AEROGEL_ARCH STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
else()
    message(FATAL_ERROR "Unknown AEROGEL_ARCH '${AEROGEL_ARCH}'. Valid: x86, x86_64")
endif()

project(aerogel VERSION 0.1.0 LANGUAGES C CXX ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(AEROGEL_COMPILE_FLAGS
    -ffreestanding
    -fno-stack-protector
    -fno-builtin
    -nostdlib
    -nostdinc
    -Wall
    -Wextra
    -O2
)

if(AEROGEL_ARCH STREQUAL "x86")
    list(APPEND AEROGEL_COMPILE_FLAGS -m32 -fno-pic -fno-pie)
    set(AEROGEL_LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/arch/x86/kernel.ld)
elseif(AEROGEL_ARCH STREQUAL "x86_64")
    list(APPEND AEROGEL_COMPILE_FLAGS -m64 -fno-pic -fno-pie -mno-red-zone -mcmodel=kernel)
    set(AEROGEL_LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/arch/x86_64/kernel.ld)
endif()

foreach(_flag ${AEROGEL_COMPILE_FLAGS})
    add_compile_options($<$<COMPILE_LANGUAGE:C>:${_flag}>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${_flag}>)
endforeach()
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

include_directories(
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/libs/libc/include
    ${CMAKE_SOURCE_DIR}/libs/libk
)

add_subdirectory(libs)
add_subdirectory(drivers)
add_subdirectory(arch/${AEROGEL_ARCH})
add_subdirectory(kernel)
