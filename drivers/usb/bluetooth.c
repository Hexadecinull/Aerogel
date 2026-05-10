#include "bluetooth.h"
#include <pci.h>
#include <kprintf.h>

#define BT_CLASS    0x0E
#define BT_SUBCLASS 0x00

static bool bt_found = false;

bool bluetooth_detect(void) {
    const pci_dev_t *d = pci_find_class(BT_CLASS, BT_SUBCLASS, 0xFF);
    if (!d) {
        kprintf("[BT] Not found via PCI (USB-attached adapters detected at runtime)\n");
        return false;
    }
    bt_found = true;
    kprintf("[BT] Adapter at PCI %02x:%02x.%x\n", d->bus, d->dev, d->fn);
    return true;
}

bool bluetooth_present(void) { return bt_found; }
