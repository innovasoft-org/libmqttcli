#ifndef __USER_DNS_H__
#define __USER_DNS_H__

#include <c_types.h>

void ICACHE_FLASH_ATTR dns_ready_cb();
void ICACHE_FLASH_ATTR dns_sent_cb(void *arg);
void ICACHE_FLASH_ATTR dns_recv_cb(void *arg, char *pdata, unsigned short len);

#endif /* __USER_DNS_H__ */