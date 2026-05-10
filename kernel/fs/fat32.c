#include "fat32.h"
#include <stdlib.h>
#include <string.h>
#include <kprintf.h>
#include <panic.h>

#define FAT_EOC 0x0FFFFFF8U
#define FAT_BAD 0x0FFFFFF7U

typedef struct {
    u32                  part_lba;
    u32                  fat_lba;
    u32                  data_lba;
    u32                  root_cluster;
    u32                  sectors_per_cluster;
    u32                  bytes_per_cluster;
    fat32_read_sector_fn read_sector;
    void                *ctx;
    u8                   sector_buf[512];
} fat32_vol_t;

typedef struct {
    fat32_vol_t *vol;
    u32          start_cluster;
    u64          size;
} fat32_file_t;

static u32 next_cluster(fat32_vol_t *v, u32 cluster) {
    u32 fat_offset = cluster * 4;
    u32 fat_sec    = v->fat_lba + fat_offset / 512;
    u32 fat_off    = fat_offset % 512;

    v->read_sector(v->ctx, fat_sec, v->sector_buf);
    u32 val;
    memcpy(&val, v->sector_buf + fat_off, 4);
    return val & 0x0FFFFFFF;
}

static u32 cluster_to_lba(fat32_vol_t *v, u32 cluster) {
    return v->data_lba + (cluster - 2) * v->sectors_per_cluster;
}

static u32 fat32_read_file(vfs_node_t *node, u64 off, u32 size, u8 *buf) {
    fat32_file_t *f = (fat32_file_t *)node->fs_data;
    fat32_vol_t  *v = f->vol;

    if (off >= f->size) return 0;
    if (off + size > f->size) size = (u32)(f->size - off);

    u32 bpc = v->bytes_per_cluster;
    u32 cluster = f->start_cluster;
    u32 off32 = (u32)off;

    u32 skip = off32 / bpc;
    for (u32 i = 0; i < skip; i++) {
        cluster = next_cluster(v, cluster);
        if (cluster >= FAT_EOC) return 0;
    }

    u32 read_total = 0;
    u32 in_cluster = off32 % bpc;

    while (read_total < size && cluster < FAT_EOC) {
        u32 lba = cluster_to_lba(v, cluster);

        u32 avail = bpc - in_cluster;
        u32 chunk = (size - read_total < avail) ? size - read_total : avail;

        u32 sec_off  = in_cluster / 512;
        u32 byte_off = in_cluster % 512;
        u32 wrote    = 0;

        while (wrote < chunk) {
            v->read_sector(v->ctx, lba + sec_off, v->sector_buf);
            u32 take = 512 - byte_off;
            if (take > chunk - wrote) take = chunk - wrote;
            memcpy(buf + read_total + wrote, v->sector_buf + byte_off, take);
            wrote += take;
            sec_off++;
            byte_off = 0;
        }

        read_total += chunk;
        in_cluster  = 0;
        cluster     = next_cluster(v, cluster);
    }
    return read_total;
}

static void parse_83(const fat32_dirent_t *de, char *out) {
    int ni = 0;
    for (int i = 0; i < 8 && de->name[i] != ' '; i++)
        out[ni++] = (char)de->name[i];
    if (de->ext[0] != ' ') {
        out[ni++] = '.';
        for (int i = 0; i < 3 && de->ext[i] != ' '; i++)
            out[ni++] = (char)de->ext[i];
    }
    out[ni] = 0;
}

static void parse_lfn(fat32_lfn_t **lfns, int count, char *out) {
    int pos = 0;
    for (int i = count - 1; i >= 0 && pos < 255; i--) {
        fat32_lfn_t *l = lfns[i];
        for (int j = 0; j < 5  && pos < 255 && l->name1[j] && l->name1[j] != 0xFFFF; j++)
            out[pos++] = (char)(l->name1[j] & 0xFF);
        for (int j = 0; j < 6  && pos < 255 && l->name2[j] && l->name2[j] != 0xFFFF; j++)
            out[pos++] = (char)(l->name2[j] & 0xFF);
        for (int j = 0; j < 2  && pos < 255 && l->name3[j] && l->name3[j] != 0xFFFF; j++)
            out[pos++] = (char)(l->name3[j] & 0xFF);
    }
    out[pos] = 0;
}

typedef struct {
    fat32_vol_t *vol;
    u32          cluster;
    vfs_node_t **entries;
    u32          count;
    bool         loaded;
} fat32_dir_t;

static void load_dir(fat32_dir_t *dir);

static vfs_node_t *fat32_finddir(vfs_node_t *node, const char *name) {
    fat32_dir_t *d = (fat32_dir_t *)node->fs_data;
    if (!d->loaded) load_dir(d);
    for (u32 i = 0; i < d->count; i++) {
        if (!strcmp(d->entries[i]->name, name)) return d->entries[i];
    }
    return NULL;
}

static vfs_node_t *fat32_readdir(vfs_node_t *node, u32 idx) {
    fat32_dir_t *d = (fat32_dir_t *)node->fs_data;
    if (!d->loaded) load_dir(d);
    return idx < d->count ? d->entries[idx] : NULL;
}

static vfs_node_t *make_dir_node(fat32_vol_t *v, const char *name, u32 cluster);

static void load_dir(fat32_dir_t *dir) {
    fat32_vol_t *v       = dir->vol;
    u32          cluster = dir->cluster;
    u32          bpc     = v->bytes_per_cluster;

    u8 *buf = (u8 *)kmalloc(bpc);

    fat32_lfn_t *lfns[20];
    int          nlfn = 0;

    dir->entries = NULL;
    dir->count   = 0;
    u32 cap = 0;

    while (cluster < FAT_EOC) {
        u32 lba = cluster_to_lba(v, cluster);
        for (u32 s = 0; s < v->sectors_per_cluster; s++) {
            v->read_sector(v->ctx, lba + s, v->sector_buf);
            fat32_dirent_t *entries = (fat32_dirent_t *)v->sector_buf;
            for (int i = 0; i < 16; i++) {
                fat32_dirent_t *de = &entries[i];
                if (de->name[0] == 0x00) goto done;
                if (de->name[0] == 0xE5) { nlfn = 0; continue; }
                if (de->attr == FAT_ATTR_LFN) {
                    if (nlfn < 20) {
                        lfns[nlfn] = (fat32_lfn_t *)kmalloc(sizeof(fat32_lfn_t));
                        memcpy(lfns[nlfn], de, sizeof(fat32_lfn_t));
                        nlfn++;
                    }
                    continue;
                }
                if (de->attr & FAT_ATTR_VOLID) { nlfn = 0; continue; }
                if (de->name[0] == '.') { nlfn = 0; continue; }

                vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
                memset(node, 0, sizeof(vfs_node_t));

                if (nlfn > 0) {
                    parse_lfn(lfns, nlfn, node->name);
                    for (int j = 0; j < nlfn; j++) kfree(lfns[j]);
                } else {
                    parse_83(de, node->name);
                }
                nlfn = 0;

                u32 cluster_start = ((u32)de->cluster_hi << 16) | de->cluster_lo;

                if (de->attr & FAT_ATTR_DIR) {
                    node->flags   = VFS_DIR;
                    node->fs_data = make_dir_node(v, node->name, cluster_start)->fs_data;
                    node->readdir = fat32_readdir;
                    node->finddir = fat32_finddir;
                } else {
                    fat32_file_t *f = (fat32_file_t *)kmalloc(sizeof(fat32_file_t));
                    f->vol           = v;
                    f->start_cluster = cluster_start;
                    f->size          = de->size;
                    node->flags   = VFS_FILE;
                    node->size    = de->size;
                    node->fs_data = f;
                    node->read    = fat32_read_file;
                }

                if (dir->count >= cap) {
                    cap = cap ? cap * 2 : 8;
                    dir->entries = (vfs_node_t **)krealloc(dir->entries,
                                                           cap * sizeof(vfs_node_t *));
                }
                dir->entries[dir->count++] = node;
            }
        }
        cluster = next_cluster(v, cluster);
    }
done:
    kfree(buf);
    dir->loaded = true;
}

static vfs_node_t *make_dir_node(fat32_vol_t *v, const char *name, u32 cluster) {
    fat32_dir_t *d = (fat32_dir_t *)kmalloc(sizeof(fat32_dir_t));
    memset(d, 0, sizeof(*d));
    d->vol     = v;
    d->cluster = cluster;

    vfs_node_t *n = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    memset(n, 0, sizeof(*n));
    strncpy(n->name, name, 255);
    n->flags   = VFS_DIR;
    n->fs_data = d;
    n->readdir = fat32_readdir;
    n->finddir = fat32_finddir;
    return n;
}

vfs_node_t *fat32_mount(u32 part_lba, fat32_read_sector_fn read_fn, void *ctx) {
    fat32_vol_t *v = (fat32_vol_t *)kmalloc(sizeof(fat32_vol_t));
    memset(v, 0, sizeof(*v));
    v->part_lba   = part_lba;
    v->read_sector = read_fn;
    v->ctx         = ctx;

    v->read_sector(ctx, part_lba, v->sector_buf);
    fat32_bpb_t *bpb = (fat32_bpb_t *)v->sector_buf;

    if (bpb->bytes_per_sector != 512) {
        kprintf("[FAT32] Unsupported sector size %u\n", bpb->bytes_per_sector);
        kfree(v);
        return NULL;
    }

    v->sectors_per_cluster = bpb->sectors_per_cluster;
    v->bytes_per_cluster   = v->sectors_per_cluster * 512;
    v->fat_lba             = part_lba + bpb->reserved_sectors;
    v->data_lba            = v->fat_lba + bpb->num_fats * bpb->fat_size_32;
    v->root_cluster        = bpb->root_cluster;

    char label[12];
    memcpy(label, bpb->volume_label, 11);
    label[11] = 0;

    kprintf("[FAT32] Volume: %.11s  cluster=%u bytes  root_cluster=%u\n",
            label, v->bytes_per_cluster, v->root_cluster);

    return make_dir_node(v, "fat32", v->root_cluster);
}
