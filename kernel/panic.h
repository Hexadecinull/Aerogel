#pragma once

#include <types.h>

NORETURN void kpanic(const char *fmt, ...);

#define ASSERT(cond) \
    do { if (!(cond)) kpanic("Assertion failed: %s at %s:%d", #cond, __FILE__, __LINE__); } while(0)
