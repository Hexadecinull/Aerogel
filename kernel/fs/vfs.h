#pragma once
#include <types.h>

#define VFS_FILE       0x01
#define VFS_DIR        0x02
#define VFS_CHARDEV    0x04
#define VFS_BLKDEV     0x08
#define VFS_MOUNTPOINT 0x40

#define VFS_O_RDONLY 0x00
#define VFS_O_WRONLY 0x01
#define VFS_O_RDWR   0x02
#define VFS_O_CREAT  0x40

struct vfs_node;

typedef u32 (*vfs_read_fn )(struct vfs_node *n, u64 off, u32 sz, u8 *buf);
typedef u32 (*vfs_write_fn)(struct vfs_node *n, u64 off, u32 sz, const u8 *buf);
typedef struct vfs_node *(*vfs_readdir_fn)(struct vfs_node *n, u32 idx);
typedef struct vfs_node *(*vfs_finddir_fn)(struct vfs_node *n, const char *name);
typedef void (*vfs_open_fn )(struct vfs_node *n, u32 flags);
typedef void (*vfs_close_fn)(struct vfs_node *n);

typedef struct vfs_node {
    char          name[256];
    u32           flags;
    u64           size;
    void         *fs_data;
    vfs_read_fn   read;
    vfs_write_fn  write;
    vfs_readdir_fn readdir;
    vfs_finddir_fn finddir;
    vfs_open_fn   open;
    vfs_close_fn  close;
    struct vfs_node *mount;
} vfs_node_t;

void       vfs_init(void);
int        vfs_mount(const char *path, vfs_node_t *root);
vfs_node_t *vfs_open(const char *path, u32 flags);
void       vfs_close(vfs_node_t *node);
u32        vfs_read(vfs_node_t *node, u64 offset, u32 size, u8 *buf);
u32        vfs_write(vfs_node_t *node, u64 offset, u32 size, const u8 *buf);
vfs_node_t *vfs_readdir(vfs_node_t *node, u32 index);
vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name);
