#pragma once

#include <stdint.h>

typedef struct {
    volatile uint32_t locked;
} spinlock_t;

#define SPINLOCK_INIT { .locked = 0 }

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
int  spinlock_try_acquire(spinlock_t *lock);
