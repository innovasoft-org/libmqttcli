#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#include "user_net.h"
#include "user_log.h"
#include "user_cfg.h"

static void ICACHE_FLASH_ATTR default_udp_sent_cb(void *arg);
static void ICACHE_FLASH_ATTR default_udp_recv_cb(void *arg, char *pdata, unsigned short len);
static void ICACHE_FLASH_ATTR default_udp_ready_cb();
static void ICACHE_FLASH_ATTR default_tcp_sent_cb(void *arg);
static void ICACHE_FLASH_ATTR default_tcp_recv_cb(void *arg, char *pdata, unsigned short len);
static void ICACHE_FLASH_ATTR default_tcp_connect_cb(void *arg);
static void ICACHE_FLASH_ATTR default_tcp_recon_cb(void *arg, sint8 err);
static void ICACHE_FLASH_ATTR default_tcp_discon_cb(void *arg);
static void ICACHE_FLASH_ATTR default_wifi_disconnected_cb(void *arg);
static uint16_t ICACHE_FLASH_ATTR net_set_softap();
static uint16_t ICACHE_FLASH_ATTR net_set_station();

static void ICACHE_FLASH_ATTR softap_monitor_cb(void *arg);
static void ICACHE_FLASH_ATTR station_monitor_cb(void *arg);

static os_timer_t system_timer;
static volatile uint8 retry_counter;
static esp_udp espudp;
static esp_tcp esptcp;
static struct espconn tcp_conn;
static struct espconn udp_conn;

static espconn_recv_callback udp_recv_callback = default_udp_recv_cb;
static espconn_sent_callback udp_sent_callback = default_udp_sent_cb;
static espconn_ready_callback udp_ready_callback = default_udp_ready_cb;
static espconn_connect_callback tcp_connect_callback = default_tcp_connect_cb;
static espconn_connect_callback tcp_disconnect_callback = default_tcp_discon_cb;
static espconn_reconnect_callback tcp_reconnect_callback = default_tcp_recon_cb;
static espconn_recv_callback tcp_recv_callback = default_tcp_recv_cb;
static espconn_sent_callback tcp_sent_callback = default_tcp_sent_cb;
static wifi_disconnected_callback wifi_disconnected_cb = default_wifi_disconnected_cb;

/**
 * @brief Registers callback for receive action in UDP protocol
 * 
 * @param[in] recv_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_udp_regist_recv_cb(espconn_recv_callback recv_callback) {
  udp_recv_callback = recv_callback;
}

/**
 * @brief Registers callback for sent action in UDP protocol
 * 
 * @param[in] sent_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_udp_regist_sent_cb(espconn_sent_callback sent_callback) {
  udp_sent_callback = sent_callback;
}

/**
 * @brief Registers callback for ready action in UDP protocol
 * It informs that send and receive actions could be performed.
 * 
 * @param[in] ready_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_udp_regist_ready_cb(espconn_ready_callback ready_callback) {
  udp_ready_callback = ready_callback;
}

/**
 * @brief Registers callback for connect action in TCP protocol
 * 
 * @param[in] connect_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_tcp_regist_connect_cb(espconn_connect_callback connect_callback) {
  tcp_connect_callback = connect_callback;
}

/**
 * @brief Registers callback for disconnect action in TCP protocol
 * 
 * @param[in] disconnect_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_tcp_regist_discon_cb(espconn_connect_callback disconnect_callback) {
  tcp_disconnect_callback = disconnect_callback;
}

/**
 * @brief Registers callback for reconnect action in TCP protocol
 * 
 * @param[in] reconnect_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_tcp_regist_recon_cb(espconn_reconnect_callback reconnect_callback) {
  tcp_reconnect_callback = reconnect_callback;
}

/**
 * @brief Registers callback for receive action in TCP protocol
 * 
 * @param[in] recv_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_tcp_regist_recv_cb(espconn_recv_callback recv_callback) {
  tcp_recv_callback = recv_callback;
}

/**
 * @brief Registers callback for sent action in TCP protocol
 * 
 * @param[in] sent_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_tcp_regist_sent_cb(espconn_sent_callback sent_callback) {
  tcp_sent_callback = sent_callback;
}

/**
 * @brief Registers callback for Wi-Fi disconnection.
 * 
 * @param[in] sent_callback Callback to be registered
 */
void ICACHE_FLASH_ATTR net_regist_wifi_disconnected_cb(wifi_disconnected_callback extern_wifi_disconnected_cb) {
  wifi_disconnected_cb = extern_wifi_disconnected_cb;
}

/**
 * @brief Default callback for sent action in UDP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_udp_sent_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_udp_sent_cb()\r\n");
}

/**
 * @brief Default callback for receive action in UDP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] pdata Received data
 * @param[in] len Received data length
 */
static void ICACHE_FLASH_ATTR default_udp_recv_cb(void *arg, char *pdata, unsigned short len) {
  TOLOG(LOG_DEBUG, "default_udp_recv_cb()\r\n");
}

/**
 * @brief Default callback for ready action in UDP protocol
 */
static void ICACHE_FLASH_ATTR default_udp_ready_cb() {
  TOLOG(LOG_DEBUG, "default_udp_ready_cb\r\n");
}

/**
 * @brief Default callback for sent action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_sent_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_sent_cb()\r\n");
}

/**
 * @brief Default callback for receive action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] pdata Received data
 * @param[in] len Received data length
 */
static void ICACHE_FLASH_ATTR default_tcp_recv_cb(void *arg, char *pdata, unsigned short len) {
  TOLOG(LOG_DEBUG, "default_tcp_recv_cb()\r\n");
}

/**
 * @brief Default callback for connect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_connect_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_connect_cb()\r\n");

  espconn_regist_recvcb( &tcp_conn, tcp_recv_callback);
  espconn_regist_sentcb( &tcp_conn, tcp_sent_callback);
  espconn_regist_disconcb( &tcp_conn, tcp_disconnect_callback);
  
  if( tcp_connect_callback != default_tcp_connect_cb )
    tcp_connect_callback( arg );
}

/**
 * @brief Default callback for reconnect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] err Error
 */
static void ICACHE_FLASH_ATTR default_tcp_recon_cb(void *arg, sint8 err) {
  uint8_t data[16] = { 0 };
  TOLOG(LOG_DEBUG, "default_tcp_recon_cb()\r\n");
  switch( err ) {
    case ESPCONN_TIMEOUT:
      TOLOG(LOG_DEBUG, "ESPCONN_TIMEOUT\r\n");
      break;
    case ESPCONN_ABRT:
      TOLOG(LOG_DEBUG, "ESPCONN_ABRT\r\n");
      break;
    case ESPCONN_RST:
      TOLOG(LOG_DEBUG, "ESPCONN_RST\r\n");
      break;
    case ESPCONN_CLSD:
      TOLOG(LOG_DEBUG, "ESPCONN_CLSD\r\n");
      break;
    case ESPCONN_CONN:
      TOLOG(LOG_DEBUG, "ESPCONN_CONN\r\n");
      break;
    case ESPCONN_HANDSHAKE:
      TOLOG(LOG_DEBUG, "ESPCONN_HANDSHAKE\r\n");
      break;
    default:
      os_sprintf(data, "err=%d\r\n", err);
      TOLOG(LOG_DEBUG, data);
  }
}

/**
 * @brief Default callback for disconnect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_discon_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_discon_cb()\r\n");
  os_timer_disarm(&system_timer);
  os_timer_setfn(&system_timer, (os_timer_func_t *)softap_monitor_cb, NULL);
  os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);
}

/**
 * @brief Default callback for Wi-Fi disconnected.
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_wifi_disconnected_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_wifi_disconnected_cb()\r\n");
}

/**
 * @brief Sends data to many devices using multicast
 * 
 * @param[in] data Pointer to data to send
 * @param[in] len Length of the data
 * @return FUN_OK on success, otherwise: FUN_W_SENDING, FUN_E_INTERNAL, FUN_E_ARGS
 */
uint16_t net_udp_send_group(const uint8_t *data, size_t len) {
  TOLOG(LOG_DEBUG,"net_udp_send_group()\r\n");

  if( ESPCONN_UDP != udp_conn.type) {
    TOLOG(LOG_CRIT,"Unsupported conn type\r\n");
    return FUN_E_ARGS;
  }

  /* Sending the message */
  udp_conn.proto.udp->remote_port = MULTICAST_PORT;
  udp_conn.proto.udp->remote_ip[0] = MULTICAST_IP0;
  udp_conn.proto.udp->remote_ip[1] = MULTICAST_IP1;
  udp_conn.proto.udp->remote_ip[2] = MULTICAST_IP2;
  udp_conn.proto.udp->remote_ip[3] = MULTICAST_IP3;

  if( espconn_sendto(&udp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "espconn_sendto() failed\r\n");
    return FUN_E_INTERNAL;
  }

  return FUN_OK;
}

/**
 * @brief Sends data only to one device using UDP protocol
 * 
 * @param[in] data Pointer to data to send
 * @param[in] len Length of the data
 * @param[in] ip IP address of the remote device
 * @return FUN_OK on success, otherwise: FUN_W_SENDING, FUN_E_INTERNAL, FUN_E_ARGS
 */
uint16_t net_udp_send(const uint8_t *data, size_t len, uint32_t ip) {
  char buffer[32];
  int i;

  TOLOG(LOG_DEBUG,"net_udp_send()\r\n");

  if( ESPCONN_UDP != udp_conn.type) {
    TOLOG(LOG_CRIT,"Unsupported conn type\r\n");
    return FUN_E_ARGS;
  }

  if( !ip ) {
    TOLOG(LOG_ERR, "ip = 0\r\n");
    return FUN_E_ARGS;
  }

  /* Sending the message */
  udp_conn.proto.udp->remote_port = MULTICAST_PORT;
  udp_conn.proto.udp->remote_ip[0] = ((ip    ) & 0xFF);
  udp_conn.proto.udp->remote_ip[1] = ((ip>> 8) & 0xFF);
  udp_conn.proto.udp->remote_ip[2] = ((ip>>16) & 0xFF);
  udp_conn.proto.udp->remote_ip[3] = ((ip>>24) & 0xFF);
  os_sprintf(buffer, "remote_ip = %d.%d.%d.%d\r\nremote_port = %d\r\n",
  udp_conn.proto.udp->remote_ip[0],
  udp_conn.proto.udp->remote_ip[1],
  udp_conn.proto.udp->remote_ip[2],
  udp_conn.proto.udp->remote_ip[3],
  udp_conn.proto.udp->remote_port);
  TOLOG(LOG_DEBUG, buffer);

  os_sprintf(buffer, "len = %d\r\n", len);
  TOLOG(LOG_DEBUG, buffer);

  for(i=0; i<len; ++i) {
    os_sprintf(buffer, "%02X ", data[i]);
    TOLOG(LOG_DEBUG, buffer);
  }
  TOLOG(LOG_DEBUG, "\r\n");

  if( espconn_sendto(&udp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "espconn_sendto() failed\r\n");
    return FUN_E_INTERNAL;
  }

  return FUN_OK;
}

/**
 * @brief Sends data using TCP protocol
 * 
 * @param[in] data Pointer to data to send
 * @param[in] len Length of the data
 * @return FUN_OK on success, otherwise: FUN_E_INTERNAL, FUN_E_ARGS
 */
uint16_t net_tcp_send(const uint8_t *data, size_t len) {
  uint8_t i;
  TOLOG(LOG_DEBUG,"net_tcp_send()\r\n");

  if( ESPCONN_TCP != tcp_conn.type) {
    TOLOG(LOG_CRIT,"Unsupported conn type\r\n");
    return FUN_E_ARGS;
  }

  if(NULL == data || 0 == len) {
    TOLOG(LOG_ERR,"Invalid args\r\n");
    return FUN_E_ARGS;
  }

  if( espconn_send( &tcp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "espconn_send() failed\r\n");
    return FUN_E_INTERNAL;
  }
}

/**
 * @brief Performs ip casting from device specific communication structure into uint32_t
 * 
 * @param[in] arg Communication parameters
 * @return IP address as uint32_t type, otherwise 0 on error
 */
uint32_t net_ip_cast(void *arg) {
  struct espconn *pesp_conn = NULL;
  remot_info *premote = NULL;

  TOLOG(LOG_DEBUG, "net_ip_cast()\r\n");

  if( !arg ) {
    TOLOG(LOG_ERR, "arg = NULL\r\n");
    return 0;
  }

  pesp_conn = (struct espconn*) arg;

  if( ESPCONN_OK != espconn_get_connection_info(pesp_conn, &premote, 0) ) {
    TOLOG(LOG_ERR, "espconn_get_connection_info() failed\r\n");
    return 0;
  }

  return  premote->remote_ip[0] | (premote->remote_ip[1]<<8) | (premote->remote_ip[2]<<16) | (premote->remote_ip[3]<<24);
}

void wifi_handle_event_cb(System_Event_t *evt) {
  switch(evt->event) {
    case EVENT_STAMODE_CONNECTED:
      TOLOG(LOG_INFO, "EVENT_STAMODE_CONNECTED\r\n");
      break;
    case EVENT_STAMODE_DISCONNECTED:
      TOLOG(LOG_INFO, "EVENT_STAMODE_DISCONNECTED\r\n");
      wifi_disconnected_cb(&evt->event_info.disconnected.reason);
      break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
      TOLOG(LOG_INFO, "EVENT_STAMODE_AUTHMODE_CHANGE\r\n");
      break;
    case EVENT_STAMODE_GOT_IP:
      TOLOG(LOG_INFO, "EVENT_STAMODE_GOT_IP\r\n");
      break;
    case EVENT_SOFTAPMODE_STACONNECTED:
      TOLOG(LOG_INFO, "EVENT_SOFTAPMODE_STACONNECTED\r\n");
      break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
      TOLOG(LOG_INFO, "EVENT_SOFTAPMODE_STADISCONNECTED\r\n");
      wifi_disconnected_cb(NULL);
      break;
    default:
      TOLOG(LOG_ERR, "Unknown event\r\n");
      break;
  }
}

LOCAL void ICACHE_FLASH_ATTR server_listen(void *arg)
{
  struct espconn *pesp_conn = arg;

  espconn_regist_recvcb(pesp_conn, tcp_recv_callback );
  espconn_regist_sentcb(pesp_conn, tcp_sent_callback );
  espconn_regist_reconcb(pesp_conn, tcp_reconnect_callback);
  espconn_regist_disconcb(pesp_conn, tcp_disconnect_callback);
}

/**
 * @brief Performs access point configuration
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR softap_monitor_cb(void *arg) {
  struct ip_info ipconfig;

  TOLOG(LOG_DEBUG, "configure_connection()\r\n");

  /* Disarm timer */
  os_timer_disarm(&system_timer);

  /* Get IP info */
  if( false == wifi_get_ip_info(SOFTAP_IF, &ipconfig) ) {
    TOLOG(LOG_ERR, "wifi_get_ip_info() failed\r\n");
    goto retry;
  }

  if( !ipconfig.ip.addr ) {
    TOLOG(LOG_ERR, "ipconfig.ip.addr: 0\r\n");
    goto retry;
  }

  /* Configure TCP/IP WWW server on port 80 */
  tcp_conn.proto.tcp->local_port = 80;
  if( 0 != espconn_regist_connectcb( &tcp_conn, server_listen) ) {
    TOLOG(LOG_ERR, "espconn_regist_connectcb()\r\n");
    goto retry;
  }
  if( 0 != espconn_accept( &tcp_conn ) ) {
    TOLOG(LOG_ERR, "espconn_accept()\r\n");
    goto retry;
  }

  TOLOG(LOG_DEBUG, "Listening...\r\n");

  /* Success */
  return;

retry:
  if( retry_counter ) {
    /* try once again */
    os_timer_setfn(&system_timer, (os_timer_func_t *) softap_monitor_cb, NULL);
    os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);
    --retry_counter;
    return;
  }
  /* Reset global variables */
  retry_counter = MAX_RETRY_CHECK_IP;
  /* Start new connection */
  net_set_softap();
}

uint16 ICACHE_FLASH_ATTR net_tcp_connect() {
  struct ip_info ipconfig;
  extern struct user_cfg cfg;
  ip_addr_t remote_addr;
  uint8_t buffer[128] = { 0 };
  err_t err;

  /* Get IP info */
  if( false == wifi_get_ip_info(STATION_IF, &ipconfig)) {
    TOLOG(LOG_ERR, "wifi_get_ip_info() failed\r\n");
    return FUN_E_INTERNAL;
  }
  
  TOLOG(LOG_ERR, "Connecting to '");
  TOLOG(LOG_ERR, cfg.br_host);
  TOLOG(LOG_ERR, "'\r\n");
  err = espconn_gethostbyname(&tcp_conn, cfg.br_host, &remote_addr, NULL);
  if(err != ESPCONN_OK) {
    TOLOG(LOG_ERR, "espconn_gethostbyname() failed\r\n");
    return FUN_E_INTERNAL;
  }
        
  os_sprintf(buffer, "remote_ip = %d.%d.%d.%d\r\nremote_port = %d\r\n",
    (remote_addr.addr      ) & 0xFF,
    (remote_addr.addr >>  8) & 0xFF,
    (remote_addr.addr >> 16) & 0xFF,
    (remote_addr.addr >> 24) & 0xFF,
    cfg.br_port);
  TOLOG(LOG_DEBUG, buffer);
  tcp_conn.proto.tcp->local_port = espconn_port();
  tcp_conn.proto.tcp->remote_port = cfg.br_port;
  tcp_conn.proto.tcp->remote_ip[0] = (remote_addr.addr      ) & 0xFF;
  tcp_conn.proto.tcp->remote_ip[1] = (remote_addr.addr >>  8) & 0xFF;
  tcp_conn.proto.tcp->remote_ip[2] = (remote_addr.addr >> 16) & 0xFF;
  tcp_conn.proto.tcp->remote_ip[3] = (remote_addr.addr >> 24) & 0xFF;
  /* Switch on the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 0);
  if( espconn_regist_connectcb( &tcp_conn, default_tcp_connect_cb) ) {
    TOLOG(LOG_ERR, "espconn_connect() failed\r\n");
    return FUN_E_INTERNAL;
  }
  if( espconn_regist_reconcb( &tcp_conn, tcp_reconnect_callback) ) {
    TOLOG(LOG_ERR, "espconn_connect() failed\r\n");
    return FUN_E_INTERNAL;
  }
  if( espconn_connect( &tcp_conn) ) {
    TOLOG(LOG_ERR, "espconn_connect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  return FUN_OK;
}

void ICACHE_FLASH_ATTR net_tcp_disconnect() {
  espconn_disconnect( &tcp_conn);
}

/**
 * @brief Monitors whether station was connected to the router.
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR station_monitor_cb(void *arg) {
  struct ip_info ipconfig;
  ip_addr_t group;
  ip_addr_t local;
  sint8 err;

  TOLOG(LOG_DEBUG, "check_ip_monitor_cb()\r\n");

  /* Disarm timer */
  os_timer_disarm(&system_timer);

  /* Switch on/off the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, !GPIO_INPUT_GET(13));

  /* Get IP info */
  if( false == wifi_get_ip_info(STATION_IF, &ipconfig)) {
    TOLOG(LOG_ERR, "wifi_get_ip_info() failed\r\n");
    goto retry;
  }

  /* If valid IP was retrievd */
  switch( wifi_station_get_connect_status() ) {
    case STATION_GOT_IP:
    {
      TOLOG(LOG_INFO, "connect status: STATION_GOT_IP\r\n");
      if( !ipconfig.ip.addr ) {
        TOLOG(LOG_ERR, "ipconfig.ip.addr = 0\r\n");
        goto retry;
      }

      if( ESPCONN_UDP == udp_conn.type ) {
        udp_conn.proto.udp->local_port = MULTICAST_PORT;
        udp_conn.proto.udp->local_ip[0] = (ipconfig.ip.addr      ) & 0xFF;
        udp_conn.proto.udp->local_ip[1] = (ipconfig.ip.addr >>  8) & 0xFF;
        udp_conn.proto.udp->local_ip[2] = (ipconfig.ip.addr >> 16) & 0xFF;
        udp_conn.proto.udp->local_ip[3] = (ipconfig.ip.addr >> 24) & 0xFF;
        udp_conn.proto.udp->remote_port = MULTICAST_PORT;
        udp_conn.proto.udp->remote_ip[0] = MULTICAST_IP0;
        udp_conn.proto.udp->remote_ip[1] = MULTICAST_IP1;
        udp_conn.proto.udp->remote_ip[2] = MULTICAST_IP2;
        udp_conn.proto.udp->remote_ip[3] = MULTICAST_IP3;
        espconn_regist_recvcb(&udp_conn, udp_recv_callback);
        espconn_regist_sentcb(&udp_conn, udp_sent_callback);
        local.addr = ipconfig.ip.addr;
        group.addr = MULTICAST_IP0 | (MULTICAST_IP1<<8) | (MULTICAST_IP2<<16) | (MULTICAST_IP3<<24);
        if( 0 != espconn_igmp_join( &local, &group ) ) {
          TOLOG(LOG_ERR, "espconn_igmp_join() failed\r\n");
          goto retry;
        }
        err = espconn_create( &udp_conn );
        if( ESPCONN_ISCONN == err) {
          TOLOG(LOG_ERR, "ESPCONN_ISCONN;\r\n");
          goto retry;
        }
        else if( ESPCONN_MEM == err) {
          TOLOG(LOG_ERR, "ESPCONN_MEM;\r\n");
          goto retry;
        }
        else if( ESPCONN_ARG == err) {
          TOLOG(LOG_ERR, "ESPCONN_ARG;\r\n");
          goto retry;
        }
        udp_ready_callback();
      }
      if( ESPCONN_TCP == tcp_conn.type ) {
        if( FUN_OK != net_tcp_connect() ) {
          goto retry;
        }
      }
      else {
        TOLOG(LOG_CRIT, "Unsupported conn type\r\n");
        return;
      }
      /* Signal that communication is possible now */
      wifi_set_event_handler_cb( wifi_handle_event_cb );
      /* Success */
      retry_counter = 0;
      return;
    }
    case STATION_WRONG_PASSWORD:
      TOLOG(LOG_INFO, "connect status: STATION_WRONG_PASSWORD\r\n");
      goto restart;
    case STATION_NO_AP_FOUND:
      TOLOG(LOG_INFO, "connect status: STATION_NO_AP_FOUND\r\n");
      goto restart;
    case STATION_CONNECT_FAIL:
      TOLOG(LOG_INFO, "connect status: STATION_CONNECT_FAILED\r\n");
      goto retry;
    case STATION_CONNECTING:
      TOLOG(LOG_INFO, "connect status: STATION_CONNECTING\r\n");
      goto retry;
    case STATION_IDLE:
      TOLOG(LOG_INFO, "connect status: STATION_IDLE\r\n");
      goto retry;
    default:
      TOLOG(LOG_ERR, "connect status: UNKNOWN\r\n");
      goto restart;
  }

retry:
  if( retry_counter ) {
    /* try once again */
    os_timer_setfn(&system_timer, (os_timer_func_t *) station_monitor_cb, NULL);
    os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);
    --retry_counter;
    return;
  }
restart:
  /* Disconnect Wi-Fi */
  wifi_station_disconnect();
  /* Reset global variables */
  retry_counter = MAX_RETRY_CHECK_IP;
  /* Start new connection */
  net_set_station();

  return;
}

/**
 * @brief Closes communication interface
 */
void ICACHE_FLASH_ATTR net_close() {
  struct ip_info ipconfig;
  ip_addr_t group;
  ip_addr_t local;

  /* disarm timer */
  os_timer_disarm(&system_timer);

  /* Get IP info */
  if( false == wifi_get_ip_info(STATION_IF, &ipconfig)) {
    TOLOG(LOG_ERR, "wifi_get_ip_info() failed\r\n");
    return;
  }

  if( ESPCONN_UDP == udp_conn.type ) {
    local.addr = ipconfig.ip.addr;
    group.addr = MULTICAST_IP0 | (MULTICAST_IP1<<8) | (MULTICAST_IP2<<16) | (MULTICAST_IP3<<24);
    if( 0 != espconn_igmp_leave( &local, &group )) {
      TOLOG(LOG_ERR, "espconn_igmp_leave() failed\r\n");
    }
  }
}

/**
 * @brief Sets device as station - it connects to the router
 * 
 * @return FUN_OK o success, otherwise: FUN_E_INTERNAL, FUN_E_ARGS
 */
static uint16_t ICACHE_FLASH_ATTR net_set_station() {
  extern struct user_cfg cfg;
  struct station_config st_config;
  char* host_name = NULL;

  TOLOG(LOG_DEBUG, "net_set_station()\r\n");

  /* Initialize Station connection */
  if( true != wifi_set_opmode_current( STATION_MODE )) {
    TOLOG(LOG_ERR, "wifi_set_opmode() failed\r\n");
    return FUN_E_INTERNAL;
  }
  if( true != wifi_station_get_config( &st_config )) {
    TOLOG(LOG_ERR, "wifi_station_get_config() failed\r\n");
    return FUN_E_INTERNAL;
  }
  os_memset(&st_config, 0x00, sizeof( struct station_config ));
  st_config.bssid_set = 0; // need not check MAC address of AP
  os_memcpy(&st_config.ssid, &cfg.wifi_ssid[0], cfg.wifi_ssid_len );
  os_memcpy(&st_config.password, &cfg.wifi_pass[0], cfg.wifi_pass_len );
  host_name = wifi_station_get_hostname();

  if( host_name == NULL && true != wifi_station_set_hostname( (char*) &cfg.dev_id[0] )) {
    TOLOG(LOG_ERR, "wifi_station_set_hostname() failed\r\n");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_set_auto_connect( 0 )) {
    TOLOG(LOG_ERR, "wifi_station_set_auto_connect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_set_config_current( &st_config )) {
    TOLOG(LOG_ERR, "wifi_station_set_config() failed\r\n");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_disconnect()) {
    TOLOG(LOG_ERR, "wifi_station_disconnect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_connect()) {
    TOLOG(LOG_ERR, "wifi_station_connect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  /* Configure timers */
  os_timer_disarm(&system_timer);
  os_timer_setfn(&system_timer, (os_timer_func_t *)station_monitor_cb, NULL);
  os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);

  /* Success */
  return FUN_OK;
}

/**
 * @brief Sets the device as access point
 * 
 * @return S_OK on success, otherwise: FUN_E_INTERNAL, FUN_E_ARGS
 */
static uint16_t ICACHE_FLASH_ATTR net_set_softap() {
  extern struct user_cfg cfg;
  struct softap_config  ap_config;

  TOLOG(LOG_DEBUG, "net_set_softap()\r\n");

  /* Initialize Access Point connection */
  if( true != wifi_set_opmode_current( SOFTAP_MODE )) {
    TOLOG(LOG_ERR, "wifi_set_opmode() failed\r\n");
    return FUN_E_INTERNAL;
  }
  if( true != wifi_softap_get_config( &ap_config )) {
    TOLOG(LOG_ERR, "wifi_softap_get_config() failed\r\n");
    return FUN_E_INTERNAL;    
  }
  /* Set device id as access point ssid */
  os_memset(&ap_config, 0x00, sizeof( struct softap_config ));
  ap_config.ssid_len = cfg.dev_id_len;
  os_memcpy(&ap_config.ssid, &cfg.dev_id[0], cfg.dev_id_len);
  os_memcpy(&ap_config.password, &cfg.ap_pass[0], cfg.ap_pass_len);
  ap_config.channel = 1;
  ap_config.authmode = AUTH_OPEN;
  ap_config.ssid_hidden = 0;
  ap_config.max_connection = 4;
  ap_config.beacon_interval = 100;
  if( true != wifi_softap_set_config_current( &ap_config )) {
    TOLOG(LOG_ERR, "wifi_softap_set_config() failed\r\n");
    return FUN_E_INTERNAL;
  }

  /* Configure timers */
  os_timer_disarm(&system_timer);
  os_timer_setfn(&system_timer, (os_timer_func_t *)softap_monitor_cb, NULL);
  os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);

  /* Success */
  return FUN_OK;
}

/**
 * @brief Initializes the network connection
 * 
 * @param[in] connection_mode Connection mode
 * @param[in] connection_type Connection type
 * @return S_OK on success, otherwise: FUN_E_ARGS
 */

uint16 ICACHE_FLASH_ATTR net_init(uint8 connection_mode, uint8 connection_type) {
  uint16_t result = FUN_OK;

  TOLOG(LOG_DEBUG, "net_init()\r\n");

  /* Reset global variables */
  retry_counter = MAX_RETRY_CHECK_IP;

  tcp_conn.type = udp_conn.type = ESPCONN_NONE;
  tcp_conn.state = udp_conn.state = ESPCONN_NONE;
  if( ESPCONN_TCP == (connection_type & ESPCONN_TCP) ) {
    tcp_conn.type = ESPCONN_TCP;
    tcp_conn.proto.tcp = &esptcp;  
  }
  if( ESPCONN_UDP == (connection_type & ESPCONN_UDP) ) {
    udp_conn.type = ESPCONN_UDP;
    udp_conn.proto.udp = &espudp;  
  }
  if ( ESPCONN_TCP != tcp_conn.type && ESPCONN_UDP != udp_conn.type ) {
    TOLOG(LOG_CRIT, "Unsupported conn type\r\n");
    return FUN_E_ARGS;
  }

  if( STATION_MODE == connection_mode ) {
    if( FUN_OK != (result = net_set_station() )) {
      return result;
    }
    TOLOG(LOG_INFO, "STATION_MODE started\r\n");
  }
  else if( SOFTAP_MODE == connection_mode) {
    if( FUN_OK != (result = net_set_softap() ) ) {
      return result;
    }
    TOLOG(LOG_INFO, "SOFTAP_MODE started\r\n");
  }
  else {
    TOLOG(LOG_CRIT, "Unsupported conn mode\r\n");
    return FUN_E_ARGS;
  }

  /* Success */
  return FUN_OK;
}
