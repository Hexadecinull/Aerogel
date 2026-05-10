#pragma once
#include <types.h>

#define PCI_MAX_DEVICES 64
#define PCI_VENDOR_NONE 0xFFFF

typedef struct {
    u8  bus, dev, fn;
    u16 vendor_id, device_id;
    u8  class_code, subclass, prog_if, revision;
    u32 bar[6];
    u8  irq_line;
} pci_dev_t;

void          pci_init(void);
u32           pci_read(u8 bus, u8 dev, u8 fn, u8 offset);
void          pci_write(u8 bus, u8 dev, u8 fn, u8 offset, u32 val);
int           pci_count(void);
const pci_dev_t *pci_get(int idx);
const pci_dev_t *pci_find_class(u8 class, u8 subclass, u8 prog_if);
