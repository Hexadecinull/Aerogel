#pragma once
#include <types.h>

#define ATA_PRIMARY_BASE   0x1F0
#define ATA_PRIMARY_ALT    0x3F6
#define ATA_SECONDARY_BASE 0x170
#define ATA_SECONDARY_ALT  0x376

#define ATA_REG_DATA       0
#define ATA_REG_ERROR      1
#define ATA_REG_SECCOUNT   2
#define ATA_REG_LBA_LO     3
#define ATA_REG_LBA_MID    4
#define ATA_REG_LBA_HI     5
#define ATA_REG_DRIVE      6
#define ATA_REG_STATUS     7
#define ATA_REG_CMD        7

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

#define ATA_CMD_READ_PIO  0x20
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_IDENTIFY  0xEC

#define ATA_MAX_DRIVES 4

typedef struct {
    bool    present;
    u16     base;
    u16     alt;
    u8      slave;
    u32     sectors;
    char    model[41];
} ata_drive_t;

int            ata_init(void);
const ata_drive_t *ata_drive(u8 idx);
int            ata_read(u8 drive, u32 lba, u8 count, void *buf);
int            ata_write(u8 drive, u32 lba, u8 count, const void *buf);
