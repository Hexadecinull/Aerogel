#include "heap.h"
#include <string.h>
#include <panic.h>

#define HEAP_MAGIC 0xAE0CA001U
#define MIN_SPLIT  32

typedef struct block_hdr {
    u32  magic;
    u32  size;
    u8   used;
    u8   _pad[3];
    struct block_hdr *next;
    struct block_hdr *prev;
} block_hdr_t;

static block_hdr_t *heap_head = NULL;
static size_t       heap_used = 0;

void heap_init(uptr start, uptr size) {
    block_hdr_t *b = (block_hdr_t *)start;
    b->magic = HEAP_MAGIC;
    b->size  = (u32)(size - sizeof(block_hdr_t));
    b->used  = 0;
    b->next  = NULL;
    b->prev  = NULL;
    heap_head = b;
    heap_used = 0;
}

static block_hdr_t *find_free(size_t size) {
    for (block_hdr_t *b = heap_head; b; b = b->next)
        if (!b->used && b->size >= size) return b;
    return NULL;
}

static void split(block_hdr_t *b, size_t size) {
    if (b->size < size + sizeof(block_hdr_t) + MIN_SPLIT) return;
    block_hdr_t *nb = (block_hdr_t *)((u8 *)b + sizeof(block_hdr_t) + size);
    nb->magic = HEAP_MAGIC;
    nb->size  = b->size - size - sizeof(block_hdr_t);
    nb->used  = 0;
    nb->next  = b->next;
    nb->prev  = b;
    if (b->next) b->next->prev = nb;
    b->next   = nb;
    b->size   = (u32)size;
}

static void coalesce(block_hdr_t *b) {
    while (b->next && !b->next->used) {
        b->size += sizeof(block_hdr_t) + b->next->size;
        b->next  = b->next->next;
        if (b->next) b->next->prev = b;
    }
    if (b->prev && !b->prev->used) {
        b->prev->size += sizeof(block_hdr_t) + b->size;
        b->prev->next  = b->next;
        if (b->next) b->next->prev = b->prev;
    }
}

void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = ALIGN_UP(size, 8);
    block_hdr_t *b = find_free(size);
    if (!b) kpanic("kmalloc: out of heap memory (requested %u bytes)", (u32)size);
    split(b, size);
    b->used    = 1;
    heap_used += size;
    return (void *)((u8 *)b + sizeof(block_hdr_t));
}

void *kcalloc(size_t n, size_t size) {
    void *p = kmalloc(n * size);
    if (p) memset(p, 0, n * size);
    return p;
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_hdr_t *b = (block_hdr_t *)((u8 *)ptr - sizeof(block_hdr_t));
    if (b->magic != HEAP_MAGIC) kpanic("kfree: corrupt heap block at %p", ptr);
    if (!b->used) kpanic("kfree: double free at %p", ptr);
    heap_used -= b->size;
    b->used    = 0;
    coalesce(b);
}

void *krealloc(void *ptr, size_t size) {
    if (!ptr)  return kmalloc(size);
    if (!size) { kfree(ptr); return NULL; }
    block_hdr_t *b = (block_hdr_t *)((u8 *)ptr - sizeof(block_hdr_t));
    if (b->magic != HEAP_MAGIC) kpanic("krealloc: corrupt heap block at %p", ptr);
    if (b->size >= size) return ptr;
    void *nb = kmalloc(size);
    memcpy(nb, ptr, b->size);
    kfree(ptr);
    return nb;
}

size_t kheap_used(void) { return heap_used; }
