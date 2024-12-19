#ifndef __USER_NET_H__
#define __USER_NET_H__

#include <espconn.h>

typedef void (* espconn_ready_callback) ();
typedef void (* wifi_disconnected_callback) (void *arg);

uint16 ICACHE_FLASH_ATTR net_init(uint8 connection_mode, uint8 connection_type);
void ICACHE_FLASH_ATTR net_close();

void ICACHE_FLASH_ATTR net_udp_regist_recv_cb(espconn_recv_callback recv_callback);
void ICACHE_FLASH_ATTR net_udp_regist_sent_cb(espconn_sent_callback sent_callback);
void ICACHE_FLASH_ATTR net_udp_regist_ready_cb(espconn_ready_callback ready_callback);

void ICACHE_FLASH_ATTR net_tcp_regist_connect_cb(espconn_connect_callback connect_callback);
void ICACHE_FLASH_ATTR net_tcp_regist_discon_cb(espconn_connect_callback disconnect_callback);
void ICACHE_FLASH_ATTR net_tcp_regist_recon_cb(espconn_reconnect_callback reconnect_callback);
void ICACHE_FLASH_ATTR net_tcp_regist_recv_cb(espconn_recv_callback recv_callback);
void ICACHE_FLASH_ATTR net_tcp_regist_sent_cb(espconn_sent_callback sent_callback);
uint16 ICACHE_FLASH_ATTR net_tcp_connect();
void ICACHE_FLASH_ATTR net_tcp_disconnect();

void ICACHE_FLASH_ATTR net_regist_wifi_disconnected_cb(wifi_disconnected_callback extern_wifi_disconnected_callback);

uint16_t net_tcp_send(const uint8_t *data, size_t len);
uint16_t net_udp_send_group(const uint8_t *data, size_t len);
uint16_t net_udp_send(const uint8_t *data, size_t len, uint32_t ip);
uint32_t net_ip_cast(void *arg);

#endif // __USER_NET_H__

