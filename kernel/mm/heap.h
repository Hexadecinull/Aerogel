#pragma once
#include <types.h>

void  heap_init(uptr start, uptr size);
void *kmalloc(size_t size);
void *kcalloc(size_t n, size_t size);
void *krealloc(void *ptr, size_t size);
void  kfree(void *ptr);
size_t kheap_used(void);
