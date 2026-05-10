#pragma once
#include <types.h>

extern uptr __stack_chk_guard;
void __attribute__((noreturn)) __stack_chk_fail(void);
