#ifndef __USER_SERVER_H__
#define __USER_SERVER_H__

#include <espconn.h>

void ICACHE_FLASH_ATTR server_sent_cb(void *arg);
void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *pdata, unsigned short len);

#endif /* __USER_SERVER_H__ */