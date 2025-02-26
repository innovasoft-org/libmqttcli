#include "stdlib.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#include "user_net.h"
#include "user_dns.h"
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

static void ICACHE_FLASH_ATTR decrement_retry_counter();

static void ICACHE_FLASH_ATTR net_reconnect_cb(void *arg);
static void ICACHE_FLASH_ATTR softap_monitor_cb(void *arg);
static void ICACHE_FLASH_ATTR station_monitor_cb(void *arg);

static os_timer_t system_timer;
static volatile uint8 retry_counter;
static uint8_t connection_mode;
static esp_udp espudp;
static esp_tcp esptcp;
static struct espconn tcp_conn, udp_conn;
static uint16_t udp_local_port, udp_remote_port, udp_local_ip[4], udp_remote_ip[4];

#define MULTICAST_DNS_IP0 ((uint8_t) 224)
#define MULTICAST_DNS_IP1 ((uint8_t) 0)
#define MULTICAST_DNS_IP2 ((uint8_t) 0)
#define MULTICAST_DNS_IP3 ((uint8_t) 251)
#define MULTICAST_DNS_PORT ((uint16_t) 5353)
#define REMOTE_DNS_PORT ((uint16_t) 53)

const uint8_t DNS_LOCAL_SUFFIX[] = ".local";

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
 * @brief Increments retry counter
 */
static void ICACHE_FLASH_ATTR decrement_retry_counter() {
  extern struct user_cfg cfg;

  /* Disarm idle timer */
  os_timer_disarm(&system_timer);

  if( retry_counter ) {
    --retry_counter;
    /* try once again */
    if(STATION_MODE == connection_mode ) {
      os_timer_setfn(&system_timer, (os_timer_func_t *) station_monitor_cb, NULL);
    }
    else if(SOFTAP_MODE == connection_mode ) {
      os_timer_setfn(&system_timer, (os_timer_func_t *) softap_monitor_cb, NULL);
    }
    else {
      system_restart();
    }
    os_timer_arm(&system_timer, DELAY_NET_MONITOR, 0);
    return;
  }

  /* Reset global variables */
  retry_counter = MAX_RETRY_CHECK_IP;

  /* Start new connection */
  if(STATION_MODE == connection_mode ) {
    if( FUN_OK != net_set_station() ) {
      system_restart();
    }
  }
  else if(SOFTAP_MODE == connection_mode ) {
    if( FUN_OK != net_set_softap() ) {
      system_restart();
    }
  }
  else {
    system_restart();
  }
}

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
 * It informs that send and receive operations could be performed.
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
  TOLOG(LOG_DEBUG, "default_udp_sent_cb()");
}

/**
 * @brief Default callback for receive action in UDP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] pdata Received data
 * @param[in] len Received data length
 */
static void ICACHE_FLASH_ATTR default_udp_recv_cb(void *arg, char *pdata, unsigned short len) {
  TOLOG(LOG_DEBUG, "default_udp_recv_cb()");
}

/**
 * @brief Default callback for ready action in UDP protocol
 */
static void ICACHE_FLASH_ATTR default_udp_ready_cb() {
  TOLOG(LOG_DEBUG, "default_udp_ready_cb()");
}

/**
 * @brief Default callback for sent action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_sent_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_sent_cb()");
}

/**
 * @brief Default callback for receive action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] pdata Received data
 * @param[in] len Received data length
 */
static void ICACHE_FLASH_ATTR default_tcp_recv_cb(void *arg, char *pdata, unsigned short len) {
  TOLOG(LOG_DEBUG, "default_tcp_recv_cb()");
}

/**
 * @brief Default callback for connect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_connect_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_connect_cb()");

  espconn_regist_recvcb( &tcp_conn, tcp_recv_callback);
  espconn_regist_sentcb( &tcp_conn, tcp_sent_callback);
  espconn_regist_disconcb( &tcp_conn, tcp_disconnect_callback);
  
  if( tcp_connect_callback != default_tcp_connect_cb ) {
    tcp_connect_callback( arg );
  }
}

/**
 * @brief Default callback for reconnect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 * @param[in] err Error
 */
static void ICACHE_FLASH_ATTR default_tcp_recon_cb(void *arg, sint8 err) {
  TOLOG(LOG_DEBUG, "default_tcp_recon_cb()");
  switch( err ) {
    case ESPCONN_TIMEOUT:
      TOLOG(LOG_DEBUG, "ESPCONN_TIMEOUT");
      break;
    case ESPCONN_ABRT:
      TOLOG(LOG_DEBUG, "ESPCONN_ABRT");
      break;
    case ESPCONN_RST:
      TOLOG(LOG_DEBUG, "ESPCONN_RST");
      break;
    case ESPCONN_CLSD:
      TOLOG(LOG_DEBUG, "ESPCONN_CLSD");
      break;
    case ESPCONN_CONN:
      TOLOG(LOG_DEBUG, "ESPCONN_CONN");
      break;
    case ESPCONN_HANDSHAKE:
      TOLOG(LOG_DEBUG, "ESPCONN_HANDSHAKE");
      break;
    default:
      TOLOG(LOG_CRIT, "");
      break;
  }
}

/**
 * @brief Default callback for disconnect action in TCP protocol
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_tcp_discon_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_tcp_discon_cb()");
  /* Force to reconnect */
  retry_counter = 0;
  decrement_retry_counter();
}

/**
 * @brief Default callback for Wi-Fi disconnected.
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR default_wifi_disconnected_cb(void *arg) {
  TOLOG(LOG_DEBUG, "default_wifi_disconnected_cb()");
}

/**
 * @brief Sends data only to specified device using UDP protocol
 * 
 * @param[in] data Pointer to data to send
 * @param[in] len Length of the data
 * @param[in] ip IP address of the remote device
 * @param[in] port port of the remote device
 * @return FUN_OK on success, otherwise: FUN_W_SENDING, FUN_E_INTERNAL, FUN_E_ARGS
 */
uint16_t net_udp_sendto(const uint8_t *data, size_t len, uint32_t ip, int port) {
  char buffer[32];
  int i;

  TOLOG(LOG_DEBUG,"net_udp_send()");

  if( ESPCONN_UDP != udp_conn.type) {
    TOLOG(LOG_CRIT,"");
    return FUN_E_ARGS;
  }

  /* Sending the message */
  udp_conn.proto.udp->remote_port = port;
  udp_conn.proto.udp->remote_ip[0] = ((ip    ) & 0xFF);
  udp_conn.proto.udp->remote_ip[1] = ((ip>> 8) & 0xFF);
  udp_conn.proto.udp->remote_ip[2] = ((ip>>16) & 0xFF);
  udp_conn.proto.udp->remote_ip[3] = ((ip>>24) & 0xFF);
  os_sprintf(buffer, "ip = %d.%d.%d.%d, port = %d",
  udp_conn.proto.udp->remote_ip[0],
  udp_conn.proto.udp->remote_ip[1],
  udp_conn.proto.udp->remote_ip[2],
  udp_conn.proto.udp->remote_ip[3],
  udp_conn.proto.udp->remote_port);
  TOLOG(LOG_DEBUG, buffer);

  os_sprintf(buffer, "len = %d", len);
  TOLOG(LOG_DEBUG, buffer);

  //for(i=0; i<len; ++i) {
  //  os_sprintf(buffer, "%02X ", data[i]);
  //  TOLOG(LOG_DEBUG, buffer);
  //}
  //TOLOG(LOG_DEBUG, "\r\n");

  if( espconn_sendto(&udp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  return FUN_OK;
}

/**
 * @brief Sends data only to specified device using UDP protocol
 * 
 * @param[in] data Pointer to data to send
 * @param[in] len Length of the data
 * @return FUN_OK on success, otherwise: FUN_W_SENDING, FUN_E_INTERNAL, FUN_E_ARGS
 */
uint16_t net_udp_send(const uint8_t *data, size_t len) {
  char buffer[32];
  int i;

  TOLOG(LOG_DEBUG,"net_udp_send()");

  if( ESPCONN_UDP != udp_conn.type) {
    TOLOG(LOG_CRIT,"");
    return FUN_E_ARGS;
  }

  /* Sending the message */
  udp_conn.proto.udp->remote_port = udp_remote_port;
  udp_conn.proto.udp->remote_ip[0] = udp_remote_ip[0];
  udp_conn.proto.udp->remote_ip[1] = udp_remote_ip[1];
  udp_conn.proto.udp->remote_ip[2] = udp_remote_ip[2];
  udp_conn.proto.udp->remote_ip[3] = udp_remote_ip[3];
  os_sprintf(buffer, "ip = %d.%d.%d.%d, port = %d",
  udp_conn.proto.udp->remote_ip[0],
  udp_conn.proto.udp->remote_ip[1],
  udp_conn.proto.udp->remote_ip[2],
  udp_conn.proto.udp->remote_ip[3],
  udp_conn.proto.udp->remote_port);
  TOLOG(LOG_DEBUG, buffer);

  os_sprintf(buffer, "len = %d", len);
  TOLOG(LOG_DEBUG, buffer);

  //for(i=0; i<len; ++i) {
  //  os_sprintf(buffer, "%02X ", data[i]);
  //  TOLOG(LOG_DEBUG, buffer);
  //}
  //TOLOG(LOG_DEBUG, "\r\n");

  if( espconn_sendto(&udp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "");
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
  TOLOG(LOG_DEBUG,"net_tcp_send()");

  if( ESPCONN_TCP != tcp_conn.type) {
    TOLOG(LOG_CRIT,"");
    return FUN_E_ARGS;
  }

  if(NULL == data || 0 == len) {
    TOLOG(LOG_ERR,"");
    return FUN_E_ARGS;
  }

  if( espconn_send( &tcp_conn, (uint8*) data, (uint16) len) ) {
    TOLOG(LOG_ERR, "");
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

  TOLOG(LOG_DEBUG, "net_ip_cast()");

  if( !arg ) {
    TOLOG(LOG_ERR, "");
    return 0;
  }

  pesp_conn = (struct espconn*) arg;

  if( ESPCONN_OK != espconn_get_connection_info(pesp_conn, &premote, 0) ) {
    TOLOG(LOG_ERR, "");
    return 0;
  }

  return  premote->remote_ip[0] | (premote->remote_ip[1]<<8) | (premote->remote_ip[2]<<16) | (premote->remote_ip[3]<<24);
}

void wifi_handle_event_cb(System_Event_t *evt) {
  switch(evt->event) {
    case EVENT_STAMODE_CONNECTED:
      TOLOG(LOG_INFO, "EVENT_STAMODE_CONNECTED");
      break;
    case EVENT_STAMODE_DISCONNECTED:
      TOLOG(LOG_INFO, "EVENT_STAMODE_DISCONNECTED");
      wifi_disconnected_cb(&evt->event_info.disconnected.reason);
      break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
      TOLOG(LOG_INFO, "EVENT_STAMODE_AUTHMODE_CHANGE");
      break;
    case EVENT_STAMODE_GOT_IP:
      TOLOG(LOG_INFO, "EVENT_STAMODE_GOT_IP");
      break;
    case EVENT_SOFTAPMODE_STACONNECTED:
      TOLOG(LOG_INFO, "EVENT_SOFTAPMODE_STACONNECTED");
      break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
      TOLOG(LOG_INFO, "EVENT_SOFTAPMODE_STADISCONNECTED");
      wifi_disconnected_cb(NULL);
      break;
    default:
      TOLOG(LOG_ERR, "");
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

  TOLOG(LOG_DEBUG, "softap_monitor_cb()");

  /* Disarm timer */
  os_timer_disarm(&system_timer);

  /* Get IP info */
  if( false == wifi_get_ip_info(SOFTAP_IF, &ipconfig) ) {
    TOLOG(LOG_ERR, "");
    decrement_retry_counter();
  }

  if( !ipconfig.ip.addr ) {
    TOLOG(LOG_ERR, "");
    decrement_retry_counter();
  }

  /* Configure TCP/IP WWW server on port 80 */
  tcp_conn.proto.tcp->local_port = 80;
  if( 0 != espconn_regist_connectcb( &tcp_conn, server_listen) ) {
    TOLOG(LOG_ERR, "");
    decrement_retry_counter();
  }
  if( 0 != espconn_accept( &tcp_conn ) ) {
    TOLOG(LOG_ERR, "");
    decrement_retry_counter();
  }

  TOLOG(LOG_DEBUG, "Listening...");

  /* Success */
  return;
}

uint16_t ICACHE_FLASH_ATTR net_connect(ip_addr_t *remote_addr) {
  extern uint8_t big_buffer[1024];
  extern struct user_cfg cfg;
  ip_addr_t group, local;
  sint8 err;

  TOLOG(LOG_DEBUG, "net_connect()");

  /* Switch off the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 1);

  if( NULL == remote_addr && cfg.dev_ttr > 0) {
    os_timer_setfn(&system_timer, (os_timer_func_t *) net_reconnect_cb, NULL);
    os_timer_arm(&system_timer, cfg.dev_ttr, 0);
    return FUN_E_INTERNAL;
  }

  if( ESPCONN_UDP == udp_conn.type ) {
    udp_remote_port = MULTICAST_PORT;
    udp_remote_ip[0] = MULTICAST_IP0;
    udp_remote_ip[1] = MULTICAST_IP1;
    udp_remote_ip[2] = MULTICAST_IP2;
    udp_remote_ip[3] = MULTICAST_IP3;

    udp_conn.proto.udp->local_port = udp_local_port;
    udp_conn.proto.udp->local_ip[0] = udp_local_ip[0];
    udp_conn.proto.udp->local_ip[1] = udp_local_ip[1];
    udp_conn.proto.udp->local_ip[2] = udp_local_ip[2];
    udp_conn.proto.udp->local_ip[3] = udp_local_ip[3];
    udp_conn.proto.udp->remote_port = udp_remote_port;
    udp_conn.proto.udp->remote_ip[0] = udp_remote_ip[0];
    udp_conn.proto.udp->remote_ip[1] = udp_remote_ip[1];
    udp_conn.proto.udp->remote_ip[2] = udp_remote_ip[2];
    udp_conn.proto.udp->remote_ip[3] = udp_remote_ip[3];    
    espconn_regist_recvcb(&udp_conn, udp_recv_callback);
    espconn_regist_sentcb(&udp_conn, udp_sent_callback);
    local.addr = udp_local_ip[0] | (udp_local_ip[1]<<8) | (udp_local_ip[2]<<16) | (udp_local_ip[3]<<24);
    group.addr = udp_remote_ip[0] | (udp_remote_ip[1]<<8) | (udp_remote_ip[2]<<16) | (udp_remote_ip[3]<<24);
    if( 0 != espconn_igmp_join( &local, &group ) ) {
      return FUN_E_INTERNAL;
    }
    espconn_delete( &udp_conn );
    err = espconn_create( &udp_conn );
    if( ESPCONN_ISCONN == err) {
    }
    else if( ESPCONN_MEM == err) {
      return FUN_E_INTERNAL;
    }
    else if( ESPCONN_ARG == err) {
      return FUN_E_INTERNAL;
    }
    udp_ready_callback();
  }

  if( NULL == remote_addr) {
    return FUN_E_INTERNAL;
  }

  if( ESPCONN_TCP == tcp_conn.type ) {
    os_sprintf(big_buffer, "ip = %d.%d.%d.%d, port = %d",
      (remote_addr->addr      ) & 0xFF,
      (remote_addr->addr >>  8) & 0xFF,
      (remote_addr->addr >> 16) & 0xFF,
      (remote_addr->addr >> 24) & 0xFF,
      cfg.br_port);
    TOLOG(LOG_DEBUG, big_buffer);
    tcp_conn.proto.tcp->local_port = espconn_port();
    tcp_conn.proto.tcp->remote_port = cfg.br_port;
    tcp_conn.proto.tcp->remote_ip[0] = (remote_addr->addr      ) & 0xFF;
    tcp_conn.proto.tcp->remote_ip[1] = (remote_addr->addr >>  8) & 0xFF;
    tcp_conn.proto.tcp->remote_ip[2] = (remote_addr->addr >> 16) & 0xFF;
    tcp_conn.proto.tcp->remote_ip[3] = (remote_addr->addr >> 24) & 0xFF;
    if( espconn_regist_connectcb( &tcp_conn, default_tcp_connect_cb) ) {
      TOLOG(LOG_ERR, "");
      return FUN_E_INTERNAL;
    }
    if( espconn_regist_reconcb( &tcp_conn, tcp_reconnect_callback) ) {
      TOLOG(LOG_ERR, "");
      return FUN_E_INTERNAL;
    }
    err = espconn_connect( &tcp_conn);
    if( ESPCONN_ISCONN == err) {
      TOLOG(LOG_ERR, "ESPCONN_ISCONN");
    }
    else if( ESPCONN_MEM == err) {
      TOLOG(LOG_ERR, "ESPCONN_MEM");
      return FUN_E_INTERNAL;
    }
    else if( ESPCONN_RTE == err) {
      TOLOG(LOG_ERR, "ESPCONN_RTE");
      return FUN_E_INTERNAL;
    }
    else if( ESPCONN_ARG == err) {
      TOLOG(LOG_ERR, "ESPCONN_ARG");
      return FUN_E_INTERNAL;
    }
  }

  /* Signal that communication is possible now */
  wifi_set_event_handler_cb( wifi_handle_event_cb );

  /* Switch on the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 0);

  /* Success */
  return FUN_OK;
}

void ICACHE_FLASH_ATTR net_tcp_disconnect() {
  espconn_disconnect( &tcp_conn);
}

static void ICACHE_FLASH_ATTR net_reconnect_cb(void *arg) {
  /* Force reconnect */
  retry_counter = 0;
  decrement_retry_counter();
}

/**
 * @brief Monitors whether station was connected to the router.
 * 
 * @param[in] arg Communication parameters
 */
static void ICACHE_FLASH_ATTR station_monitor_cb(void *arg) {
  extern struct user_cfg cfg;
  uint8_t *p;
  size_t dns_local_suffix_len = sizeof(DNS_LOCAL_SUFFIX)/sizeof(DNS_LOCAL_SUFFIX[0]) - 1;
  struct ip_info ipconfig;
  ip_addr_t remote;
  sint8 err;

  TOLOG(LOG_DEBUG, "station_monitor_cb()");

  /* Disarm timer */
  os_timer_disarm(&system_timer);

  /* Switch on/off the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, !GPIO_INPUT_GET(13));

  /* If valid IP was retrievd */
  switch( wifi_station_get_connect_status() ) {
    case STATION_GOT_IP:
    {
      TOLOG(LOG_INFO, "STATION_GOT_IP");
      /* Switch on the led (there is negative polarization) */
      GPIO_OUTPUT_SET(13, 0);
      /* Disarm system timer */
      os_timer_disarm(&system_timer);
      retry_counter = MAX_RETRY_CHECK_IP;

      /* Get IP info */
      if( false == wifi_get_ip_info(STATION_IF, &ipconfig)) {
        TOLOG(LOG_ERR, "");
        decrement_retry_counter();
        return;
      }

      if( !ipconfig.ip.addr ) {
        TOLOG(LOG_ERR, "");
        decrement_retry_counter();
        return;
      }

      if( ESPCONN_TCP == tcp_conn.type ) {
        /* If the remote server name starts with a number */
        if(cfg.br_host[0] >= '0' && cfg.br_host[0] <= '9') {
          err = espconn_gethostbyname(&tcp_conn, cfg.br_host, &remote, NULL);
          if(err != ESPCONN_OK) {
            TOLOG(LOG_ERR, "");
            decrement_retry_counter();
            return;
          }
          if( FUN_OK != net_connect( &remote ) ) {
            decrement_retry_counter();
            return;
          }
        }
        else {
          p = &cfg.br_host[ cfg.br_host_len - dns_local_suffix_len];

          udp_local_port = MULTICAST_PORT;
          udp_local_ip[0] = (ipconfig.ip.addr      ) & 0xFF;
          udp_local_ip[1] = (ipconfig.ip.addr >>  8) & 0xFF;
          udp_local_ip[2] = (ipconfig.ip.addr >> 16) & 0xFF;
          udp_local_ip[3] = (ipconfig.ip.addr >> 24) & 0xFF;

          if( 0 == memcmp(p, DNS_LOCAL_SUFFIX, dns_local_suffix_len)) {
            /* Multicast DNS configuration */
            udp_remote_port = MULTICAST_DNS_PORT;
            udp_remote_ip[0] = MULTICAST_DNS_IP0;
            udp_remote_ip[1] = MULTICAST_DNS_IP1;
            udp_remote_ip[2] = MULTICAST_DNS_IP2;
            udp_remote_ip[3] = MULTICAST_DNS_IP3;
          }
          else {
            /* DNS configuration */
            udp_remote_port = REMOTE_DNS_PORT;
            udp_local_ip[0] = (ipconfig.gw.addr      ) & 0xFF;
            udp_local_ip[1] = (ipconfig.gw.addr >>  8) & 0xFF;
            udp_local_ip[2] = (ipconfig.gw.addr >> 16) & 0xFF;
            udp_local_ip[3] = (ipconfig.gw.addr >> 24) & 0xFF;
          }
          udp_conn.proto.udp->local_port = udp_local_port;
          udp_conn.proto.udp->local_ip[0] = udp_local_ip[0];
          udp_conn.proto.udp->local_ip[1] = udp_local_ip[1];
          udp_conn.proto.udp->local_ip[2] = udp_local_ip[2];
          udp_conn.proto.udp->local_ip[3] = udp_local_ip[3];
          udp_conn.proto.udp->remote_port = udp_remote_port;
          udp_conn.proto.udp->remote_ip[0] = udp_remote_ip[0];
          udp_conn.proto.udp->remote_ip[1] = udp_remote_ip[1];
          udp_conn.proto.udp->remote_ip[2] = udp_remote_ip[2];
          udp_conn.proto.udp->remote_ip[3] = udp_remote_ip[3];
          espconn_regist_recvcb(&udp_conn, dns_recv_cb);
          espconn_regist_sentcb(&udp_conn, dns_sent_cb);
          
          espconn_delete( &udp_conn );
          err = espconn_create( &udp_conn );
          if( ESPCONN_ISCONN == err) { 
          }
          else if( ESPCONN_MEM == err) {
            decrement_retry_counter();
            return;
          }
          else if( ESPCONN_ARG == err) {
            decrement_retry_counter();
            return;
          }
          dns_ready_cb();
        }
      }
      break;
    }
    case STATION_WRONG_PASSWORD:
      TOLOG(LOG_INFO, "STATION_WRONG_PASSWORD");
      retry_counter = 0;
      decrement_retry_counter();
      return;
    case STATION_NO_AP_FOUND:
      TOLOG(LOG_INFO, "STATION_NO_AP_FOUND");
      retry_counter = 0;
      decrement_retry_counter();
      return;
    case STATION_CONNECT_FAIL:
      TOLOG(LOG_INFO, "STATION_CONNECT_FAILED");
      decrement_retry_counter();
      return;
    case STATION_CONNECTING:
      TOLOG(LOG_INFO, "STATION_CONNECTING");
      decrement_retry_counter();
      return;
    case STATION_IDLE:
      TOLOG(LOG_INFO, "STATION_IDLE");
      decrement_retry_counter();
      return;
    default:
      TOLOG(LOG_ERR, "");
      retry_counter = 0;
      decrement_retry_counter();
      return;
  }

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
    TOLOG(LOG_ERR, "");
    return;
  }

  if( ESPCONN_UDP == udp_conn.type ) {
    local.addr = ipconfig.ip.addr;
    group.addr = MULTICAST_IP0 | (MULTICAST_IP1<<8) | (MULTICAST_IP2<<16) | (MULTICAST_IP3<<24);
    if( 0 != espconn_igmp_leave( &local, &group )) {
      TOLOG(LOG_ERR, "");
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

  TOLOG(LOG_DEBUG, "net_set_station()");

  /* Initialize Station connection */
  if( true != wifi_set_opmode_current( STATION_MODE )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }
  if( true != wifi_station_get_config( &st_config )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }
  os_memset(&st_config, 0x00, sizeof( struct station_config ));
  st_config.bssid_set = 0; // need not check MAC address of AP
  os_memcpy(&st_config.ssid, &cfg.wifi_ssid[0], cfg.wifi_ssid_len );
  os_memcpy(&st_config.password, &cfg.wifi_pass[0], cfg.wifi_pass_len );
  host_name = wifi_station_get_hostname();

  if( host_name == NULL && true != wifi_station_set_hostname( (char*) &cfg.dev_id[0] )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_set_auto_connect( 0 )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_set_config_current( &st_config )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_disconnect()) {
    TOLOG(LOG_ERR, "wifi_station_disconnect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  if( true != wifi_station_connect()) {
    TOLOG(LOG_ERR, "");
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

  TOLOG(LOG_DEBUG, "net_set_softap()");

  /* Initialize Access Point connection */
  if( true != wifi_set_opmode_current( SOFTAP_MODE )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }
  if( true != wifi_softap_get_config( &ap_config )) {
    TOLOG(LOG_ERR, "");
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
    TOLOG(LOG_ERR, "");
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
 * @param[in] __connection_mode Connection mode
 * @param[in] __connection_type Connection type
 * @return S_OK on success, otherwise: FUN_E_ARGS
 */

uint16_t ICACHE_FLASH_ATTR net_init(uint8_t __connection_mode, uint8_t __connection_type) {
  uint16_t result = FUN_OK;

  TOLOG(LOG_DEBUG, "net_init()");

  /* Reset global variables */
  retry_counter = MAX_RETRY_CHECK_IP;

  tcp_conn.type = udp_conn.type = ESPCONN_NONE;
  tcp_conn.state = udp_conn.state = ESPCONN_NONE;
  if( ESPCONN_TCP == (__connection_type & ESPCONN_TCP) ) {
    tcp_conn.type = ESPCONN_TCP;
    tcp_conn.proto.tcp = &esptcp;  
  }
  if( ESPCONN_UDP == (__connection_type & ESPCONN_UDP) ) {
    udp_conn.type = ESPCONN_UDP;
    udp_conn.proto.udp = &espudp;  
  }
  if ( ESPCONN_TCP != tcp_conn.type && ESPCONN_UDP != udp_conn.type ) {
    TOLOG(LOG_CRIT, "");
    return FUN_E_ARGS;
  }

  connection_mode = __connection_mode;

  if( STATION_MODE == connection_mode ) {
    if( FUN_OK != (result = net_set_station() )) {
      return result;
    }
  }
  else if( SOFTAP_MODE == connection_mode) {
    if( FUN_OK != (result = net_set_softap() ) ) {
      return result;
    }
  }
  else {
    TOLOG(LOG_CRIT, "");
    return FUN_E_ARGS;
  }

  /* Success */
  return FUN_OK;
}
