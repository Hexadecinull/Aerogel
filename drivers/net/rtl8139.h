#pragma once
#include <types.h>

#define RTL_VENDOR  0x10EC
#define RTL_DEVICE  0x8139

typedef void (*rtl_rx_cb_t)(const u8 *data, u32 len, void *ctx);

bool rtl8139_init(rtl_rx_cb_t cb, void *ctx);
bool rtl8139_present(void);
int  rtl8139_send(const u8 *data, u32 len);
void rtl8139_get_mac(u8 mac[6]);
void rtl8139_poll(void);
