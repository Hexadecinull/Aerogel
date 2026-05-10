add_library(aerogel_drivers STATIC
    serial/serial.c
    timer/pit.c
    display/vga_text.c
    display/framebuffer.c
    input/ps2_keyboard.c
    input/ps2_mouse.c
    storage/ata.c
    bus/pci.c
    usb/xhci.c
    usb/bluetooth.c
    net/rtl8139.c
)

target_include_directories(aerogel_drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/serial
    ${CMAKE_CURRENT_SOURCE_DIR}/timer
    ${CMAKE_CURRENT_SOURCE_DIR}/display
    ${CMAKE_CURRENT_SOURCE_DIR}/input
    ${CMAKE_CURRENT_SOURCE_DIR}/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/bus
    ${CMAKE_CURRENT_SOURCE_DIR}/usb
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_SOURCE_DIR}/arch/${AEROGEL_ARCH}/cpu
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/mm
    ${CMAKE_SOURCE_DIR}/libs/libc/include
    ${CMAKE_SOURCE_DIR}/libs/libk
)

target_link_libraries(aerogel_drivers PRIVATE aerogel_libc aerogel_libk)
