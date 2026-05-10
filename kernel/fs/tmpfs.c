#include "tmpfs.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    vfs_node_t **entries;
    u32          count;
    u32          cap;
} tmpfs_dir_t;

typedef struct {
    u8  *data;
    u32  size;
} tmpfs_file_t;

static u32 tmpfs_read(vfs_node_t *node, u64 off, u32 sz, u8 *buf) {
    tmpfs_file_t *f = (tmpfs_file_t *)node->fs_data;
    if (off >= f->size) return 0;
    if (off + sz > f->size) sz = (u32)(f->size - off);
    memcpy(buf, f->data + (u32)off, sz);
    return sz;
}

static u32 tmpfs_write(vfs_node_t *node, u64 off, u32 sz, const u8 *buf) {
    tmpfs_file_t *f = (tmpfs_file_t *)node->fs_data;
    if ((u32)off + sz > f->size) {
        f->data = (u8 *)krealloc(f->data, (size_t)((u32)off + sz));
        f->size = (u32)off + sz;
        node->size = f->size;
    }
    memcpy(f->data + (u32)off, buf, sz);
    return sz;
}

static vfs_node_t *tmpfs_readdir(vfs_node_t *node, u32 idx) {
    tmpfs_dir_t *d = (tmpfs_dir_t *)node->fs_data;
    return idx < d->count ? d->entries[idx] : NULL;
}

static vfs_node_t *tmpfs_finddir(vfs_node_t *node, const char *name) {
    tmpfs_dir_t *d = (tmpfs_dir_t *)node->fs_data;
    for (u32 i = 0; i < d->count; i++)
        if (!strcmp(d->entries[i]->name, name)) return d->entries[i];
    return NULL;
}

static void dir_add(tmpfs_dir_t *d, vfs_node_t *n) {
    if (d->count >= d->cap) {
        d->cap = d->cap ? d->cap * 2 : 4;
        d->entries = (vfs_node_t **)krealloc(d->entries, d->cap * sizeof(vfs_node_t *));
    }
    d->entries[d->count++] = n;
}

vfs_node_t *tmpfs_create(const char *name) {
    tmpfs_dir_t *d = (tmpfs_dir_t *)kmalloc(sizeof(tmpfs_dir_t));
    memset(d, 0, sizeof(*d));
    vfs_node_t *n = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    memset(n, 0, sizeof(*n));
    strncpy(n->name, name, 255);
    n->flags   = VFS_DIR;
    n->fs_data = d;
    n->readdir = tmpfs_readdir;
    n->finddir = tmpfs_finddir;
    return n;
}

vfs_node_t *tmpfs_mkdir(vfs_node_t *parent, const char *name) {
    vfs_node_t  *n = tmpfs_create(name);
    tmpfs_dir_t *d = (tmpfs_dir_t *)parent->fs_data;
    dir_add(d, n);
    return n;
}

vfs_node_t *tmpfs_mkfile(vfs_node_t *parent, const char *name,
                          const u8 *data, u32 size) {
    tmpfs_file_t *f = (tmpfs_file_t *)kmalloc(sizeof(tmpfs_file_t));
    f->data = (u8 *)kmalloc(size);
    f->size = size;
    if (data) memcpy(f->data, data, size);

    vfs_node_t *n = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    memset(n, 0, sizeof(*n));
    strncpy(n->name, name, 255);
    n->flags   = VFS_FILE;
    n->size    = size;
    n->fs_data = f;
    n->read    = tmpfs_read;
    n->write   = tmpfs_write;

    tmpfs_dir_t *d = (tmpfs_dir_t *)parent->fs_data;
    dir_add(d, n);
    return n;
}
