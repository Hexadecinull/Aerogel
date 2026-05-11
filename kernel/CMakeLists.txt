set(KERNEL_SOURCES
    kmain.c
    panic.c
    canary.c
    mm/pmm.c
    mm/heap.c
    fs/vfs.c
    fs/fat32.c
    fs/tmpfs.c
    sched/thread.c
    sched/sched.c
    gui/gui.c
    net/net.c
    ../userspace/shell/shell.c
    ../userspace/setup/setup.c
)

if(AEROGEL_ARCH STREQUAL "x86")
    list(APPEND KERNEL_SOURCES mm/vmm.c)
endif()

add_executable(kernel.elf ${KERNEL_SOURCES})

target_include_directories(kernel.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/mm
    ${CMAKE_CURRENT_SOURCE_DIR}/fs
    ${CMAKE_CURRENT_SOURCE_DIR}/sched
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_SOURCE_DIR}/arch/${AEROGEL_ARCH}/cpu
    ${CMAKE_SOURCE_DIR}/arch/${AEROGEL_ARCH}/mm
    ${CMAKE_SOURCE_DIR}/drivers/serial
    ${CMAKE_SOURCE_DIR}/drivers/timer
    ${CMAKE_SOURCE_DIR}/drivers/display
    ${CMAKE_SOURCE_DIR}/drivers/input
    ${CMAKE_SOURCE_DIR}/drivers/storage
    ${CMAKE_SOURCE_DIR}/drivers/bus
    ${CMAKE_SOURCE_DIR}/drivers/usb
    ${CMAKE_SOURCE_DIR}/drivers/net
    ${CMAKE_SOURCE_DIR}/userspace/shell
    ${CMAKE_SOURCE_DIR}/userspace/setup
    ${CMAKE_SOURCE_DIR}/libs/libc/include
    ${CMAKE_SOURCE_DIR}/libs/libk
)

set_target_properties(kernel.elf PROPERTIES
    LINK_DEPENDS ${AEROGEL_LINKER_SCRIPT})

target_link_options(kernel.elf PRIVATE
    -T ${AEROGEL_LINKER_SCRIPT}
    -nostdlib
    -no-pie
    -Wl,-z,max-page-size=0x1000
    -Wl,--build-id=none
)

target_link_libraries(kernel.elf PRIVATE
    aerogel_arch aerogel_drivers aerogel_libc aerogel_libk
)

# Link Rust static library if it exists (built by CI)
if(EXISTS ${CMAKE_SOURCE_DIR}/rust/target/aerogel-x86/release/libaerogel_core.a)
    target_link_libraries(kernel.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/rust/target/aerogel-x86/release/libaerogel_core.a)
    target_compile_definitions(kernel.elf PRIVATE AEROGEL_RUST=1)
elseif(EXISTS ${CMAKE_SOURCE_DIR}/rust/target/release/libaerogel_core.a)
    target_link_libraries(kernel.elf PRIVATE
        ${CMAKE_SOURCE_DIR}/rust/target/release/libaerogel_core.a)
    target_compile_definitions(kernel.elf PRIVATE AEROGEL_RUST=1)
endif()

add_custom_command(TARGET kernel.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
            ${CMAKE_CURRENT_BINARY_DIR}/kernel.elf
            ${CMAKE_BINARY_DIR}/kernel.bin
    COMMENT "kernel.elf -> kernel.bin"
)
