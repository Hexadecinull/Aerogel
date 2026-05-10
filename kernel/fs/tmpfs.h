#pragma once
#include "vfs.h"

vfs_node_t *tmpfs_create(const char *name);
vfs_node_t *tmpfs_mkfile(vfs_node_t *dir, const char *name, const u8 *data, u32 size);
vfs_node_t *tmpfs_mkdir(vfs_node_t *dir, const char *name);
