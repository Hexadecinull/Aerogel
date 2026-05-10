enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)

add_library(aerogel_arch STATIC
    cpu/entry.asm
    cpu/gdt.asm
    cpu/gdt.c
    cpu/idt.c
    cpu/isr.asm
    cpu/isr.c
    cpu/switch.asm
)

target_include_directories(aerogel_arch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/mm
    ${CMAKE_SOURCE_DIR}/kernel/sched
    ${CMAKE_SOURCE_DIR}/libs/libc/include
    ${CMAKE_SOURCE_DIR}/libs/libk
)

target_link_libraries(aerogel_arch PRIVATE aerogel_libc aerogel_libk)
