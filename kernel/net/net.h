#pragma once
#include <types.h>

#define NET_MTU     1514
#define IP_NONE     0x00000000

typedef void (*net_tx_fn_t)(const u8 *data, u32 len, void *ctx);

void net_init(void);
bool net_set_interface(net_tx_fn_t tx, void *ctx, const u8 mac[6]);
void net_configure(u32 ip, u32 netmask, u32 gateway);
void net_receive(const u8 *data, u32 len);
void net_poll(void);
u32  net_our_ip(void);
void net_get_mac(u8 mac[6]);
bool net_up(void);
void net_bring_up(void);

extern bool net_process_packet(
    const u8 *data, u32 len,
    const u8 *our_mac, u32 our_ip,
    u8 *reply_buf, u32 *reply_len);

extern u32 net_arp_announce(const u8 *our_mac, u32 our_ip, u8 *out);
