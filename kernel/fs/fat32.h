#pragma once
#include <types.h>
#include "vfs.h"

typedef struct {
    u8  jmp[3];
    u8  oem[8];
    u16 bytes_per_sector;
    u8  sectors_per_cluster;
    u16 reserved_sectors;
    u8  num_fats;
    u16 root_entry_count;
    u16 total_sectors_16;
    u8  media;
    u16 fat_size_16;
    u16 sectors_per_track;
    u16 num_heads;
    u32 hidden_sectors;
    u32 total_sectors_32;
    u32 fat_size_32;
    u16 ext_flags;
    u16 fs_version;
    u32 root_cluster;
    u16 fs_info;
    u16 backup_boot;
    u8  reserved[12];
    u8  drive_num;
    u8  reserved2;
    u8  boot_sig;
    u32 volume_id;
    u8  volume_label[11];
    u8  fs_type[8];
} PACKED fat32_bpb_t;

typedef struct {
    u8  name[8];
    u8  ext[3];
    u8  attr;
    u8  reserved;
    u8  ctime_tenths;
    u16 ctime;
    u16 cdate;
    u16 adate;
    u16 cluster_hi;
    u16 mtime;
    u16 mdate;
    u16 cluster_lo;
    u32 size;
} PACKED fat32_dirent_t;

typedef struct {
    u8  order;
    u16 name1[5];
    u8  attr;
    u8  type;
    u8  checksum;
    u16 name2[6];
    u16 first_cluster;
    u16 name3[2];
} PACKED fat32_lfn_t;

#define FAT_ATTR_READONLY 0x01
#define FAT_ATTR_HIDDEN   0x02
#define FAT_ATTR_SYSTEM   0x04
#define FAT_ATTR_VOLID    0x08
#define FAT_ATTR_DIR      0x10
#define FAT_ATTR_ARCHIVE  0x20
#define FAT_ATTR_LFN      0x0F

typedef int (*fat32_read_sector_fn)(void *ctx, u32 lba, void *buf);

vfs_node_t *fat32_mount(u32 part_lba, fat32_read_sector_fn read_fn, void *ctx);
