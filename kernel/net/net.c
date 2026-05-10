#include "net.h"
#include <rtl8139.h>
#include <string.h>
#include <kprintf.h>

static u32         our_ip   = 0;
static u8          our_mac[6];
static net_tx_fn_t tx_fn    = NULL;
static void       *tx_ctx   = NULL;
static bool        iface_up = false;

static u8 rx_reply[NET_MTU];

static void rtl_rx(const u8 *data, u32 len, void *ctx) {
    (void)ctx;
    u32 reply_len = 0;
    bool has_reply = net_process_packet(
        data, len, our_mac, our_ip, rx_reply, &reply_len);
    if (has_reply && reply_len > 0 && tx_fn)
        tx_fn(rx_reply, reply_len, tx_ctx);
}

static void rtl_tx(const u8 *data, u32 len, void *ctx) {
    (void)ctx;
    rtl8139_send(data, len);
}


/* C fallback stubs — replaced by Rust library (libaerogel_core.a) in CI builds */
__attribute__((weak))
bool net_process_packet(
    const u8 *data, u32 len,
    const u8 *our_mac, u32 our_ip,
    u8 *reply_buf, u32 *reply_len)
{
    (void)data; (void)len; (void)our_mac;
    (void)our_ip; (void)reply_buf;
    *reply_len = 0;
    return false;
}

__attribute__((weak))
u32 net_arp_announce(const u8 *our_mac, u32 our_ip, u8 *out) {
    (void)our_mac; (void)our_ip; (void)out;
    return 0;
}

void net_init(void) {
    memset(our_mac, 0, 6);
    our_ip = 0;
    iface_up = false;
}

bool net_set_interface(net_tx_fn_t tx, void *ctx, const u8 mac[6]) {
    tx_fn  = tx;
    tx_ctx = ctx;
    memcpy(our_mac, mac, 6);
    iface_up = true;
    return true;
}

void net_configure(u32 ip, u32 netmask, u32 gateway) {
    (void)netmask; (void)gateway;
    our_ip = ip;
    kprintf("[NET] IP: %u.%u.%u.%u\n",
            (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF);

    u8 ann[42];
    u32 ann_len = net_arp_announce(our_mac, our_ip, ann);
    if (ann_len > 0 && tx_fn)
        tx_fn(ann, ann_len, tx_ctx);
}

void net_receive(const u8 *data, u32 len) {
    rtl_rx(data, len, NULL);
}

void net_poll(void) {
    rtl8139_poll();
}

u32  net_our_ip(void)          { return our_ip; }
void net_get_mac(u8 mac[6])    { memcpy(mac, our_mac, 6); }
bool net_up(void)              { return iface_up; }

void net_bring_up(void) {
    if (rtl8139_init(rtl_rx, NULL)) {
        u8 mac[6];
        rtl8139_get_mac(mac);
        net_set_interface(rtl_tx, NULL, mac);
        net_configure(0xC0A80002, 0xFFFFFF00, 0xC0A80001); // 192.168.0.2
    }
}
