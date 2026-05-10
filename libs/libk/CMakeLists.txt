add_library(aerogel_libk STATIC
    kprintf.c
    spinlock.c
)

target_include_directories(aerogel_libk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(aerogel_libk PRIVATE aerogel_libc)
