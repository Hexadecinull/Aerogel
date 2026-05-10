#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uintptr_t uptr;
typedef ptrdiff_t iptr;

#define PACKED      __attribute__((packed))
#define NORETURN    __attribute__((noreturn))
#define ALIGN(n)    __attribute__((aligned(n)))
#define UNUSED      __attribute__((unused))
#define INLINE      __attribute__((always_inline)) static inline

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))
#define ALIGN_UP(x, a)   (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
