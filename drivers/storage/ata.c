#include "ata.h"
#include <io.h>
#include <string.h>
#include <kprintf.h>

static ata_drive_t drives[ATA_MAX_DRIVES];
static int ndrives = 0;

static void delay400(u16 alt) {
    for (int i = 0; i < 4; i++) inb(alt);
}

static int wait_busy(u16 base) {
    for (int i = 0; i < 100000; i++) {
        u8 s = inb(base + ATA_REG_STATUS);
        if (!(s & ATA_SR_BSY)) return (s & ATA_SR_ERR) ? -1 : 0;
    }
    return -1;
}

static int wait_drq(u16 base) {
    for (int i = 0; i < 100000; i++) {
        u8 s = inb(base + ATA_REG_STATUS);
        if (s & ATA_SR_ERR) return -1;
        if (s & ATA_SR_DRQ) return 0;
    }
    return -1;
}

static void probe(u16 base, u16 alt, u8 slave, u8 idx) {
    outb(base + ATA_REG_DRIVE, slave ? 0xB0 : 0xA0);
    delay400(alt);

    outb(base + ATA_REG_SECCOUNT, 0);
    outb(base + ATA_REG_LBA_LO,  0);
    outb(base + ATA_REG_LBA_MID, 0);
    outb(base + ATA_REG_LBA_HI,  0);
    outb(base + ATA_REG_CMD, ATA_CMD_IDENTIFY);
    delay400(alt);

    if (inb(base + ATA_REG_STATUS) == 0) return;
    if (wait_busy(base) < 0) return;

    u8 mid = inb(base + ATA_REG_LBA_MID);
    u8 hi  = inb(base + ATA_REG_LBA_HI);
    if (mid || hi) return;

    if (wait_drq(base) < 0) return;

    u16 id[256];
    for (int i = 0; i < 256; i++) id[i] = inw(base + ATA_REG_DATA);

    ata_drive_t *d = &drives[idx];
    d->present = true;
    d->base    = base;
    d->alt     = alt;
    d->slave   = slave;
    d->sectors = (u32)id[60] | ((u32)id[61] << 16);

    for (int i = 0; i < 20; i++) {
        d->model[i*2]   = (char)(id[27+i] >> 8);
        d->model[i*2+1] = (char)(id[27+i] & 0xFF);
    }
    d->model[40] = 0;
    char *end = d->model + 39;
    while (end > d->model && *end == ' ') *end-- = 0;
}

int ata_init(void) {
    probe(ATA_PRIMARY_BASE,   ATA_PRIMARY_ALT,   0, 0);
    probe(ATA_PRIMARY_BASE,   ATA_PRIMARY_ALT,   1, 1);
    probe(ATA_SECONDARY_BASE, ATA_SECONDARY_ALT, 0, 2);
    probe(ATA_SECONDARY_BASE, ATA_SECONDARY_ALT, 1, 3);

    for (int i = 0; i < ATA_MAX_DRIVES; i++) {
        if (drives[i].present) {
            kprintf("[ATA] Drive %d: %s  %u MiB\n",
                i, drives[i].model, drives[i].sectors / 2048);
            ndrives++;
        }
    }
    return ndrives;
}

const ata_drive_t *ata_drive(u8 idx) {
    return (idx < ATA_MAX_DRIVES) ? &drives[idx] : NULL;
}

int ata_read(u8 drv, u32 lba, u8 count, void *buf) {
    if (drv >= ATA_MAX_DRIVES || !drives[drv].present) return -1;
    ata_drive_t *d = &drives[drv];
    u16 *dst = (u16 *)buf;

    if (wait_busy(d->base) < 0) return -1;

    outb(d->base + ATA_REG_DRIVE,    (u8)(0xE0 | (d->slave << 4) | ((lba >> 24) & 0x0F)));
    outb(d->base + ATA_REG_SECCOUNT, count);
    outb(d->base + ATA_REG_LBA_LO,  (u8)(lba & 0xFF));
    outb(d->base + ATA_REG_LBA_MID, (u8)((lba >> 8)  & 0xFF));
    outb(d->base + ATA_REG_LBA_HI,  (u8)((lba >> 16) & 0xFF));
    outb(d->base + ATA_REG_CMD,      ATA_CMD_READ_PIO);

    for (u8 s = 0; s < count; s++) {
        if (wait_drq(d->base) < 0) return -1;
        for (int w = 0; w < 256; w++)
            dst[s * 256 + w] = inw(d->base + ATA_REG_DATA);
    }
    return 0;
}

int ata_write(u8 drv, u32 lba, u8 count, const void *buf) {
    if (drv >= ATA_MAX_DRIVES || !drives[drv].present) return -1;
    ata_drive_t *d = &drives[drv];
    const u16 *src = (const u16 *)buf;

    if (wait_busy(d->base) < 0) return -1;

    outb(d->base + ATA_REG_DRIVE,    (u8)(0xE0 | (d->slave << 4) | ((lba >> 24) & 0x0F)));
    outb(d->base + ATA_REG_SECCOUNT, count);
    outb(d->base + ATA_REG_LBA_LO,  (u8)(lba & 0xFF));
    outb(d->base + ATA_REG_LBA_MID, (u8)((lba >> 8)  & 0xFF));
    outb(d->base + ATA_REG_LBA_HI,  (u8)((lba >> 16) & 0xFF));
    outb(d->base + ATA_REG_CMD,      ATA_CMD_WRITE_PIO);

    for (u8 s = 0; s < count; s++) {
        if (wait_drq(d->base) < 0) return -1;
        for (int w = 0; w < 256; w++)
            outw(d->base + ATA_REG_DATA, src[s * 256 + w]);
        outb(d->base + ATA_REG_CMD, 0xE7);
        if (wait_busy(d->base) < 0) return -1;
    }
    return 0;
}
