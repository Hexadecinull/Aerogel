#include "rtl8139.h"
#include <pci.h>
#include <io.h>
#include <pmm.h>
#include <string.h>
#include <kprintf.h>

#define RX_BUF_SIZE  (8192 + 16 + 1500)
#define TX_BUF_SIZE  1536
#define TX_BUFS      4

#define RTL_IDR0     0x00
#define RTL_MAR0     0x08
#define RTL_TXSTATUS 0x10
#define RTL_TXADDR   0x20
#define RTL_RXBUF    0x30
#define RTL_CMD      0x37
#define RTL_CAPR     0x38
#define RTL_IMR      0x3C
#define RTL_ISR      0x3E
#define RTL_TCR      0x40
#define RTL_RCR      0x44
#define RTL_CONFIG1  0x52

#define CMD_RESET    0x10
#define CMD_RXEN     0x08
#define CMD_TXEN     0x04

#define ISR_ROK      0x0001
#define ISR_TOK      0x0004

static u16       io_base  = 0;
static u8       *rx_buf   = NULL;
static u8       *tx_bufs[TX_BUFS];
static int       tx_cur   = 0;
static u16       rx_ptr   = 0;
static bool      present  = false;
static rtl_rx_cb_t rx_cb = NULL;
static void     *rx_ctx   = NULL;
static u8        mac[6];

static inline u8  rtl_rb(u8 reg) { return inb(io_base + reg); }
static inline u16 rtl_rw(u8 reg) { return inw(io_base + reg); }
static inline u32 rtl_rl(u8 reg) { return inl(io_base + reg); }
static inline void rtl_wb(u8 reg, u8  v) { outb(io_base + reg, v); }
static inline void rtl_ww(u8 reg, u16 v) { outw(io_base + reg, v); }
static inline void rtl_wl(u8 reg, u32 v) { outl(io_base + reg, v); }

bool rtl8139_init(rtl_rx_cb_t cb, void *ctx) {
    const pci_dev_t *d = pci_find_class(0x02, 0x00, 0xFF);
    if (!d || d->vendor_id != RTL_VENDOR || d->device_id != RTL_DEVICE) {
        kprintf("[RTL8139] Not found\n");
        return false;
    }

    u32 bar0 = d->bar[0];
    if (!(bar0 & 1)) { kprintf("[RTL8139] BAR0 not I/O space\n"); return false; }
    io_base = (u16)(bar0 & ~3);

    u32 pci_cmd = pci_read(d->bus, d->dev, d->fn, 0x04);
    pci_write(d->bus, d->dev, d->fn, 0x04, pci_cmd | 0x04);

    rx_cb  = cb;
    rx_ctx = ctx;

    rtl_wb(RTL_CONFIG1, 0x00);

    rtl_wb(RTL_CMD, CMD_RESET);
    for (int i = 0; i < 10000; i++) if (!(rtl_rb(RTL_CMD) & CMD_RESET)) break;

    for (int i = 0; i < 6; i++) mac[i] = rtl_rb((u8)(RTL_IDR0 + i));

    rx_buf = (u8 *)pmm_alloc_n((RX_BUF_SIZE + 4095) / 4096);
    if (!rx_buf) return false;
    memset(rx_buf, 0, RX_BUF_SIZE);

    for (int i = 0; i < TX_BUFS; i++) {
        tx_bufs[i] = (u8 *)pmm_alloc();
        memset(tx_bufs[i], 0, 4096);
    }

    rtl_wl(RTL_RXBUF, (u32)(uptr)rx_buf);

    for (int i = 0; i < TX_BUFS; i++)
        rtl_wl((u8)(RTL_TXADDR + i*4), (u32)(uptr)tx_bufs[i]);

    rtl_ww(RTL_IMR, ISR_ROK | ISR_TOK);
    rtl_wl(RTL_RCR, 0x0F | (1 << 7));
    rtl_wl(RTL_TCR, 0x03000700);
    rtl_wb(RTL_CMD, CMD_RXEN | CMD_TXEN);

    rx_ptr = 0;
    present = true;

    kprintf("[RTL8139] MAC %02x:%02x:%02x:%02x:%02x:%02x  IO=0x%x\n",
            mac[0],mac[1],mac[2],mac[3],mac[4],mac[5], io_base);
    return true;
}

bool rtl8139_present(void) { return present; }
void rtl8139_get_mac(u8 m[6]) { memcpy(m, mac, 6); }

int rtl8139_send(const u8 *data, u32 len) {
    if (!present || len > 1500) return -1;
    memcpy(tx_bufs[tx_cur], data, len);
    rtl_wl((u8)(RTL_TXSTATUS + tx_cur*4), len);
    tx_cur = (tx_cur + 1) % TX_BUFS;
    return 0;
}

void rtl8139_poll(void) {
    if (!present) return;
    u16 isr = rtl_rw(RTL_ISR);
    if (!isr) return;
    rtl_ww(RTL_ISR, isr);

    if (isr & ISR_ROK) {
        while (!(rtl_rb(RTL_CMD) & 0x01)) {
            u16  off  = (u16)(rx_ptr % 8192);
            u8  *hdr  = rx_buf + off;
            u16  plen = (u16)(((u16)hdr[3] << 8) | hdr[2]);
            if (plen < 4 || plen > 1514) break;
            u8 *pkt  = hdr + 4;
            if (rx_cb) rx_cb(pkt, plen - 4, rx_ctx);
            rx_ptr = (u16)((rx_ptr + plen + 4 + 3) & ~3);
            rtl_ww(RTL_CAPR, (u16)(rx_ptr - 16));
        }
    }
}
