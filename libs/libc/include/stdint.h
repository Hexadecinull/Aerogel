#pragma once

typedef __UINT8_TYPE__   uint8_t;
typedef __UINT16_TYPE__  uint16_t;
typedef __UINT32_TYPE__  uint32_t;
typedef __UINT64_TYPE__  uint64_t;
typedef __INT8_TYPE__    int8_t;
typedef __INT16_TYPE__   int16_t;
typedef __INT32_TYPE__   int32_t;
typedef __INT64_TYPE__   int64_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__  intptr_t;
typedef __UINTMAX_TYPE__ uintmax_t;
typedef __INTMAX_TYPE__  intmax_t;
typedef __SIZE_TYPE__    size_t;

#define UINT8_MAX   ((uint8_t)0xFF)
#define UINT16_MAX  ((uint16_t)0xFFFF)
#define UINT32_MAX  ((uint32_t)0xFFFFFFFFU)
#define UINT64_MAX  ((uint64_t)0xFFFFFFFFFFFFFFFFULL)
#define INT8_MAX    ((int8_t)0x7F)
#define INT16_MAX   ((int16_t)0x7FFF)
#define INT32_MAX   ((int32_t)0x7FFFFFFF)
#define INT64_MAX   ((int64_t)0x7FFFFFFFFFFFFFFFLL)
#define INT8_MIN    ((int8_t)(-0x80))
#define INT16_MIN   ((int16_t)(-0x8000))
#define INT32_MIN   ((int32_t)(-0x80000000))
#define INT64_MIN   ((int64_t)(-0x8000000000000000LL))
#define SIZE_MAX    (~(size_t)0)
