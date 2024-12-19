#ifndef __USER_MQTT_H__
#define __USER_MQTT_H__

void ICACHE_FLASH_ATTR mqtt_restart_cb (void *arg);
void ICACHE_FLASH_ATTR mqtt_disconnect_cb(void *arg);
void ICACHE_FLASH_ATTR matt_reconnect_cb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR mqtt_ready_cb(void *arg);
void ICACHE_FLASH_ATTR mqtt_udp_ready_cb(void *arg);
void ICACHE_FLASH_ATTR mqtt_recv_cb(void *arg, char *pdata, unsigned short len);
void ICACHE_FLASH_ATTR mqtt_udp_recv_cb(void *arg, char *pdata, unsigned short len);
void ICACHE_FLASH_ATTR mqtt_sent_cb(void *arg);

#endif /* __USER_MQTT_H__ */