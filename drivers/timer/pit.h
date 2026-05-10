#pragma once
#include <types.h>

void pit_init(u32 hz);
u64  pit_ticks(void);
void pit_sleep_ms(u32 ms);
