#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <kprintf.h>

#define MAX_MOUNTS 16

typedef struct {
    char        path[256];
    vfs_node_t *root;
} mount_entry_t;

static mount_entry_t mounts[MAX_MOUNTS];
static int           nmounts = 0;
static vfs_node_t   *vfs_root = NULL;

void vfs_init(void) {
    nmounts  = 0;
    vfs_root = NULL;
}

int vfs_mount(const char *path, vfs_node_t *root) {
    if (nmounts >= MAX_MOUNTS) return -1;
    strncpy(mounts[nmounts].path, path, 255);
    mounts[nmounts].root = root;
    nmounts++;
    if (!vfs_root && path[0] == '/' && path[1] == 0)
        vfs_root = root;
    kprintf("[VFS] Mounted '%s' at %s\n", root->name, path);
    return 0;
}

static vfs_node_t *resolve(const char *path) {
    if (!path || path[0] != '/') return NULL;

    vfs_node_t *node = vfs_root;
    if (!node) return NULL;
    if (path[1] == 0) return node;

    char buf[256];
    strncpy(buf, path + 1, 255);

    char *tok = buf;
    while (tok && *tok) {
        char *sep = strchr(tok, '/');
        if (sep) *sep = 0;

        if (!node->finddir) return NULL;
        node = node->finddir(node, tok);
        if (!node) return NULL;

        tok = sep ? sep + 1 : NULL;
    }
    return node;
}

vfs_node_t *vfs_open(const char *path, u32 flags) {
    vfs_node_t *n = resolve(path);
    if (!n) return NULL;
    if (n->open) n->open(n, flags);
    return n;
}

void vfs_close(vfs_node_t *node) {
    if (node && node->close) node->close(node);
}

u32 vfs_read(vfs_node_t *n, u64 off, u32 sz, u8 *buf) {
    if (!n || !n->read) return 0;
    return n->read(n, off, sz, buf);
}

u32 vfs_write(vfs_node_t *n, u64 off, u32 sz, const u8 *buf) {
    if (!n || !n->write) return 0;
    return n->write(n, off, sz, buf);
}

vfs_node_t *vfs_readdir(vfs_node_t *n, u32 idx) {
    if (!n || !(n->flags & VFS_DIR) || !n->readdir) return NULL;
    return n->readdir(n, idx);
}

vfs_node_t *vfs_finddir(vfs_node_t *n, const char *name) {
    if (!n || !(n->flags & VFS_DIR) || !n->finddir) return NULL;
    return n->finddir(n, name);
}
