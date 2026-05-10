add_library(aerogel_libc STATIC
    src/string.c
    src/printf.c
    src/stdlib.c
)

target_include_directories(aerogel_libc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
