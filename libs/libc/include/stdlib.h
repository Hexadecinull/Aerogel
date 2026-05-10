#pragma once
#include <stddef.h>

void  *kmalloc (size_t size);
void  *kcalloc (size_t n, size_t size);
void  *krealloc(void *ptr, size_t size);
void   kfree   (void *ptr);
long   atol    (const char *s);
int    atoi    (const char *s);
