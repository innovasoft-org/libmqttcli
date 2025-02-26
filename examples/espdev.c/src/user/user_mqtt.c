#include <user_interface.h>
#include <c_types.h>
#include <osapi.h>

#include "../include/user_config.h"
#include "../mqttcli/mqtt_cli.h"
#include "user_mqtt.h"
#include "user_net.h"
#include "user_log.h"
#include "user_cfg.h"
#include "user_util.h"

/** Defines task handler with 0 id */
#define MQTT_HANDLER_ID   0
/** Event queue length */
#define EVENT_QUEUE_LEN   8
/** Handler signal: initialize */
#define SIG_INIT          1
/** Handler signal: data was received */
#define SIG_RX            2
/** Handler signal: timeout */
#define SIG_TIMEOUT       4
/** Handler signal close */
#define SIG_CLOSE         8

const uint8_t GPIO0[] = "gpio0";
const uint8_t GPIO2[] = "gpio2";
const uint8_t GPIO4[] = "gpio4";
const uint8_t GPIO5[] = "gpio5";
const uint8_t GPIO12[] = "gpio12";
const uint8_t GPIO13[] = "gpio13";
const uint8_t GPIO14[] = "gpio14";
const uint8_t GPIO15[] = "gpio15";

uint8_t mqtt_suback = 0;

uint8_t mqtt_counter = 0;
/** Determines if there is pending data to send */
uint8_t pending_data = 0;
/** Internal mqtt timer used to determine idle state */
os_timer_t mqtt_idle_timer;
/** Event queue */
os_event_t *mqtt_event_queue = NULL;
struct espconn *espconn;
extern const size_t big_buffer_len;
extern uint8_t big_buffer[1024];
clv_t data = { .capacity=sizeof(big_buffer)/sizeof(big_buffer[0]), .value=big_buffer };

static void ICACHE_FLASH_ATTR mqtt_handler(os_event_t *e);

void ICACHE_FLASH_ATTR mqtt_restart_cb (void *arg) {
  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  TOLOG(LOG_DEBUG, "mqtt_restart_cb()");

  /* Restart the system */
  system_restart();
}

void ICACHE_FLASH_ATTR mqtt_disconnect_cb(void *arg) {
  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  TOLOG(LOG_DEBUG, "mqtt_disconnect_cb()");

  /* Close connection */
  system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0);
}

void ICACHE_FLASH_ATTR matt_reconnect_cb(void *arg, sint8 err)  {
  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  if(espconn != NULL) {
    /* Close connection */
    system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0);
  }
  else {
    net_connect( NULL );
  }
}

static void ICACHE_FLASH_ATTR mqtt_idle_cb(void *arg) {
  /* Disarm idle timer */
  os_timer_disarm(&mqtt_idle_timer);

  TOLOG(LOG_DEBUG, "idle_cb()");

  /* Send the event */
  system_os_post(MQTT_HANDLER_ID, SIG_TIMEOUT, 0);

  /* Initialize idle timer once again */
  os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
  os_timer_arm(&mqtt_idle_timer, DELAY_1_SEC, 0);
}


void ICACHE_FLASH_ATTR mqtt_ready_cb(void *arg) {
  TOLOG(LOG_DEBUG, "mqtt_ready_cb()");

  /* Switch off the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 1);

  espconn = NULL;
  
  /* Initialize task handler */
  if( NULL == (mqtt_event_queue = (os_event_t*) malloc( sizeof(os_event_t) * EVENT_QUEUE_LEN))) {
    goto failure;
  }
  if( false == system_os_task(mqtt_handler, MQTT_HANDLER_ID, mqtt_event_queue, EVENT_QUEUE_LEN)) {
    goto failure;
  }

  /* Send the event */
  if( false == system_os_post(MQTT_HANDLER_ID, SIG_INIT, 0) ) {
    goto failure;
  }

  /** Store current connection data */
  espconn = arg;

  /* Start idle timer */
  os_timer_disarm(&mqtt_idle_timer);
  os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
  os_timer_arm(&mqtt_idle_timer, DELAY_1_SEC, 0);

  /* Switch on the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 0);

  /** Return */
  return;

failure:
  /* force to restart the system */
  mqtt_disconnect_cb(NULL);
}

void ICACHE_FLASH_ATTR mqtt_udp_ready_cb(void *arg) {
  TOLOG(LOG_DEBUG, "mqtt_udp_ready_cb()");
}

void ICACHE_FLASH_ATTR mqtt_recv_cb(void *arg, char *pdata, unsigned short len) {
  uint8_t iter;
  uint32_t event_param_id = EVENT_QUEUE_LEN;
  uint8_t tab[4] = { 0 };
  size_t i;

  TOLOG(LOG_DEBUG, "mqtt_recv_cb()");

  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  /* If the packet length is incorrect */
  if( big_buffer_len < len || 0 == len) {
    TOLOG(LOG_ERR,"recv data too big");
    return;
  }

  if(data.length > 0 ) {
    TOLOG(LOG_ERR,"data in use");
    return;
  }

  /* Copy received data */
  os_memcpy(data.value, pdata, len);
  data.length = len;

  TOLOG(LOG_ERR, "Receiving length: ");
  os_memset(tab, 0x00, sizeof(tab)/sizeof(tab[0]));
  os_sprintf(tab, "%d", data.length);
  TOLOG(LOG_ERR, tab);
  //for(i=0; i<data.length; ++i) {
  //  os_sprintf(tab, "%02x ", big_buffer[i]);
  //  TOLOG(LOG_INFO, tab);
  //}
  //TOLOG(LOG_INFO, "\r\n");

  /* Send the event */
  system_os_post(MQTT_HANDLER_ID, SIG_RX, 0);

  /* Start idle timer */
  os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
  os_timer_arm(&mqtt_idle_timer, DELAY_1_SEC, 0);
}

void ICACHE_FLASH_ATTR mqtt_udp_recv_cb(void *arg, char *pdata, unsigned short len) {
  if(len != 2) {
    return;
  }
  if(pdata[1] != 0x00) {
    return;
  }
  switch( pdata[0] ) {
    case 0xC0:
      pdata[0] = 0xD0;
      net_udp_sendto(pdata, len, net_ip_cast(arg), MULTICAST_PORT);
      break;
    case 0xF1:
      wifi_station_disconnect();
      mqtt_restart_cb( NULL );
      break;
    case 0xF2:
      cfg_set_defaults();
      cfg_save();
      mqtt_restart_cb( NULL );
      break;
  }
}

void ICACHE_FLASH_ATTR mqtt_sent_cb(void *arg) {
  TOLOG(LOG_DEBUG, "mqtt_sent_cb()");
  data.length = 0;
  if( pending_data ) {
    pending_data = 0;
    system_os_post(MQTT_HANDLER_ID, SIG_TIMEOUT, 0);
    /* Start idle timer */
    //os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
    //os_timer_arm(&mqtt_idle_timer, DELAY_1_SEC, 0);
  }
}

int ICACHE_FLASH_ATTR get_gpio_num(uint8_t* name, size_t name_len) {
  if( (name_len == ARRAYLEN(GPIO0) -1) && (0 == os_memcmp(name, GPIO0, ARRAYLEN(GPIO0) -1)) ) {
    return 0;
  }
  else if( (name_len == ARRAYLEN(GPIO2) -1) && (0 == os_memcmp(name, GPIO2, ARRAYLEN(GPIO2) -1)) ) {
    return 2;
  }
  else if( (name_len == ARRAYLEN(GPIO4) -1) && (0 == os_memcmp(name, GPIO4, ARRAYLEN(GPIO4) -1)) ) {
    return 4;
  }
  else if( (name_len == ARRAYLEN(GPIO5) -1) && (0 == os_memcmp(name, GPIO5, ARRAYLEN(GPIO5) -1)) ) {
    return 5;
  }
  else if( (name_len == ARRAYLEN(GPIO12) -1) && (0 == os_memcmp(name, GPIO12, ARRAYLEN(GPIO12) -1)) ) {
    return 12;
  }
  else if( (name_len == ARRAYLEN(GPIO13) -1) && (0 == os_memcmp(name, GPIO13, ARRAYLEN(GPIO13) -1)) ) {
    return 13;
  }
  else if( (name_len == ARRAYLEN(GPIO14) -1) && (0 == os_memcmp(name, GPIO14, ARRAYLEN(GPIO14) -1)) ) {
    return 14;
  }
  else if( (name_len == ARRAYLEN(GPIO15) -1) && (0 == os_memcmp(name, GPIO15, ARRAYLEN(GPIO15) -1)) ) {
    return 15;
  }
  else {
    return 12;
  }
}

mqtt_rc_t cb_connack(const mqtt_cli_ctx_cb_t *self, const mqtt_connack_t *pkt, const mqtt_channel_t *channel) {
  extern struct user_cfg cfg;
  mqtt_rc_t rc = RC_SUCCESS;
  uint8_t *message;
  uint8_t *ptr = NULL;
  int offset;
  mqtt_publish_params_t publish_params = { };
  mqtt_subscribe_params_t subscribe_params = { };
  
  /* Publishing configuration */
  ptr = data.value;
  publish_params.topic.value = ptr;
  if(cfg.ha_node_id_len) {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s/config", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id);
  }
  else {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/config", cfg.ha_base_t, cfg.dev_name, cfg.dev_id);
  }
  message = publish_params.message.value = ptr + publish_params.topic.length;
  offset = 0;
  message[0] = '{';
  offset += 1;
  if(cfg.ha_node_id_len) {
    offset += os_sprintf( message + offset, "\"~\": \"%s/%s/%s/%s\",", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id );
  }
  else {
    offset += os_sprintf( message + offset, "\"~\": \"%s/%s/%s\",", cfg.ha_base_t, cfg.dev_name, cfg.dev_id );
  }
  offset += os_sprintf( message + offset, "\"name\": null,");
  offset += os_sprintf( message + offset, "\"uniq_id\": \"%s\",", cfg.dev_id);
  offset += os_sprintf( message + offset, "\"cmd_t\": \"~/%s\",", cfg.ha_cmd_t);
  offset += os_sprintf( message + offset, "\"stat_t\": \"~/%s\",", cfg.ha_stat_t);
  offset += os_sprintf( message + offset, "\"avty_t\": \"~/%s\",", cfg.ha_avty_t);
  offset += os_sprintf( message + offset, "\"schema\": \"json\",");
  offset += os_sprintf( message + offset, "\"pl_on\": \"%s\",", cfg.ha_pl_on);
  offset += os_sprintf( message + offset, "\"pl_off\": \"%s\",", cfg.ha_pl_off);
  offset += os_sprintf( message + offset, "\"pl_avail\" : \"%s\",", cfg.ha_pl_avail);
  offset += os_sprintf( message + offset, "\"pl_not_avail\": \"%s\",", cfg.ha_pl_not_avail);
  offset += os_sprintf( message + offset, "\"stat_on\": \"%s\",", cfg.ha_stat_on);
  offset += os_sprintf( message + offset, "\"stat_off\": \"%s\",", cfg.ha_stat_off);
  offset += os_sprintf( message + offset, "\"ret\": \"false\",");
  offset += os_sprintf( message + offset, "\"opt\": \"false\",");
  offset += os_sprintf( message + offset, "\"dev\": {\"ids\": [\"%s\"],\"name\": \"%s\",\"mf\": \"Innovasoft\",\"mdl\": \"%s\",\"sw\": \"%s\",\"sn\": \"%s\",\"hw\": \"%s\"},", cfg.dev_id, cfg.dev_id, cfg.dev_name, cfg.dev_sw, cfg.dev_id, cfg.dev_hw);
  offset += os_sprintf( message + offset, "\"o\": {\"name\":\"ESP-OS\",\"sw\": \"%s\",\"url\": \"https://www.innovasoft.org\"}", cfg.dev_sw);
  offset += os_sprintf( message + offset, "}");
  publish_params.message.length = offset;
  if( MQTT_SUCCESS != self->publish(self, &publish_params) ) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;
  }

  /* Subscribing to receive commands */
  ptr = data.value;
  subscribe_params.filter.value = ptr;
  if(cfg.ha_node_id_len) {
    subscribe_params.filter.length = os_sprintf( ptr, "%s/%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id, cfg.ha_cmd_t );
  }
  else {
    subscribe_params.filter.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.dev_id, cfg.ha_cmd_t );
  }
  if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
    rc =  RC_IMPL_SPEC_ERR;
    goto finish;   
  }

finish:
  return rc;
}

mqtt_rc_t cb_publish(const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel) {
  extern struct user_cfg cfg;
  extern volatile int gpio_num;
  int gpio_state;
  mqtt_publish_params_t publish_params = { };
  size_t offset;
  uint8_t *ptr, *message;

  /* Checking if correct command was send */
  ptr = pkt->topic.value;
  offset = 0;
  if( 0 != os_memcmp(ptr + offset, cfg.ha_base_t, cfg.ha_base_t_len) ) {
    return RC_TOPIC_NAME_INV;
  }
  offset += cfg.ha_base_t_len;
  if(pkt->topic.value[offset] != '/') {
    return RC_TOPIC_NAME_INV;
  }
  offset += 1;
  if( 0 != os_memcmp(ptr + offset, cfg.dev_name, cfg.dev_name_len) ) {
    return RC_TOPIC_NAME_INV;
  }
  offset += cfg.dev_name_len;
  if(pkt->topic.value[offset] != '/') {
    return RC_TOPIC_NAME_INV;
  }
  offset += 1;
  if(cfg.ha_node_id_len) {
    if( 0 != os_memcmp(ptr + offset, cfg.ha_node_id, cfg.ha_node_id_len) ) {
      return RC_TOPIC_NAME_INV;
    }
    offset += cfg.ha_node_id_len;
    if(pkt->topic.value[offset] != '/') {
      return RC_TOPIC_NAME_INV;
    }
    offset += 1;
  }
  if( 0 != os_memcmp(ptr + offset, cfg.dev_id, cfg.dev_id_len) ) {
    return RC_TOPIC_NAME_INV;
  }
  offset += cfg.dev_id_len;
  if(pkt->topic.value[offset] != '/') {
    return RC_TOPIC_NAME_INV;
  }
  offset += 1;
  if( 0 != os_memcmp(ptr + offset, cfg.ha_cmd_t, cfg.ha_cmd_t_len) ) {
    return RC_TOPIC_NAME_INV;
  }

  /* Determining GPIO number */
  gpio_num = get_gpio_num(cfg.ha_cmd_t, cfg.ha_cmd_t_len);

  /* Updating device internal state */
  ptr = pkt->message.value;
  if( (pkt->message.length == 1) && (*ptr == '1') ) {
    gpio_state = 1;
  }
  else if( (pkt->message.length == 1) && (*ptr == '0') ) {
    gpio_state = 0;
  }
  else if( (pkt->message.length == cfg.ha_pl_on_len) && (0 == os_memcmp( ptr, cfg.ha_pl_on, cfg.ha_pl_on_len)) ) {
    gpio_state = 1;
  }
  else if( (pkt->message.length == cfg.ha_pl_off_len) && (0 == os_memcmp( ptr, cfg.ha_pl_off, cfg.ha_pl_off_len)) ) {
    gpio_state = 0;
  }
  else {
    return RC_PAYLOAD_INV;
  }

  /* Setting GPIO state */
  GPIO_OUTPUT_SET(gpio_num, gpio_state);

  /* Obtaining current GPIO state */
  gpio_state = GPIO_INPUT_GET( gpio_num );

  /* Publishing current state */
  ptr = &data.value[0];
  publish_params.topic.value = ptr;
  if(cfg.ha_node_id_len) {publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id, cfg.ha_stat_t );
  }
  else {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.dev_id, cfg.ha_stat_t );
  }
  message = publish_params.message.value = ptr + publish_params.topic.length;
  publish_params.message.length = os_sprintf( message, "%s", ( gpio_state > 0 ) ? cfg.ha_stat_on : cfg.ha_stat_off );
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    return RC_IMPL_SPEC_ERR;
  }

  return RC_SUCCESS;
}

void cb_suback(const mqtt_cli_ctx_cb_t *self, const mqtt_suback_t *pkt, const mqtt_channel_t *channel) {
  extern struct user_cfg cfg;
  extern volatile int gpio_num;
  int gpio_state;
  uint8_t *ptr, *message;
  mqtt_publish_params_t publish_params = { };

  /* Publishing current device availability */
  ptr = data.value;
  publish_params.topic.value = ptr;
  if(cfg.ha_node_id_len) {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id, cfg.ha_avty_t);
  }
  else {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.dev_id, cfg.ha_avty_t);
  }
  message = publish_params.message.value = ptr + publish_params.topic.length;
  publish_params.message.length = os_sprintf( message, "%s", cfg.ha_pl_avail );
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    return; 
  }

  /* Publishing current device state */
  ptr = data.value;
  publish_params.topic.value = ptr;
  if(cfg.ha_node_id_len) {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id, cfg.ha_stat_t );
  }
  else {
    publish_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.dev_id, cfg.ha_stat_t );
  }
  /* Determining GPIO */
  gpio_num = get_gpio_num(cfg.ha_stat_t, cfg.ha_stat_t_len);
  /* Obtaining current GPIO state */
  gpio_state = GPIO_INPUT_GET( gpio_num );
  message = publish_params.message.value = ptr + publish_params.topic.length;
  publish_params.message.length = os_sprintf( message, "%s", ( gpio_state > 0 ) ? cfg.ha_stat_on : cfg.ha_stat_off );
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    return;
  }

  /* Let HA to process configuration packet */
  os_delay_us( 2e6 );

  /* Switch off the led (there is negative polarization) */
  GPIO_OUTPUT_SET(13, 1);
}

static void ICACHE_FLASH_ATTR mqtt_handler(os_event_t *e) {
  extern struct user_cfg cfg;
  static mqtt_cli_t cli;
  static bool mqtt_initialized = false;
  uint16_t rc = MQTT_SUCCESS;
  mqtt_params_t mqtt_params = { .bufsize=big_buffer_len, .max_pkt_id=8, .timeout=1, .version=4 };
  mqtt_will_params_t will_params = (mqtt_will_params_t) { };
  lv_t cli_userid, cli_username, cli_password;
  uint8_t *ptr = NULL, *ptr0 = NULL, connected, *message;
  uint8_t tab[9] = { 0 };
  size_t length, i;
  sint8 err;
  mqtt_publish_params_t publish_params = { };
  int gpio_state;
  extern volatile int gpio_num;

  TOLOG(LOG_DEBUG, "mqtt_handler()");

  /* Disarm idle timer */
  os_timer_disarm(&mqtt_idle_timer);

  switch(e->sig) {
    case SIG_CLOSE:
      TOLOG(LOG_CRIT,"SIG_CLOSE");
      espconn = NULL;
      net_tcp_disconnect();
      mqtt_initialized = false;
      mqtt_cli_destr( &cli );
      if( FUN_OK != net_connect( NULL ) ) {
        goto restart;
      }
      break;
    case SIG_INIT:
      mqtt_initialized = false;      
      if( MQTT_SUCCESS != (rc = mqtt_cli_init_ex( &cli, &mqtt_params )) ) {
        TOLOG(LOG_CRIT,"");
        goto restart;
      }
      cli.set_cb_connack( &cli, cb_connack );
      cli.set_cb_publish( &cli, cb_publish );
      cli.set_cb_suback(  &cli, cb_suback );
      cli.set_br_keepalive( &cli, (uint16_t) 60);
      /** Set the Will */
      ptr0 = ptr = &big_buffer[0];
      will_params.topic.value = ptr;
      ptr = (uint8_t*) os_memcpy( ptr, cfg.ha_base_t, cfg.ha_base_t_len ) + cfg.ha_base_t_len;
      *ptr++ = '/';
      ptr = (uint8_t*) os_memcpy( ptr, cfg.ha_avty_t, cfg.ha_avty_t_len ) + cfg.ha_avty_t_len;
      will_params.topic.length = ptr - ptr0;
      ptr0 = ptr;
      will_params.payload.value = ptr;
      ptr = (uint8_t*) os_memcpy( ptr, cfg.ha_pl_not_avail, cfg.ha_pl_not_avail_len ) + cfg.ha_pl_not_avail_len;
      will_params.payload.length = ptr - ptr0;;
      if( MQTT_SUCCESS != (rc = cli.set_br_will( &cli, &will_params) ) ) {
        TOLOG(LOG_ERR,"");
        goto restart;
      }
      /** Set UserID */
      if( cfg.br_userid_len ) {
        cli_userid.length = cfg.br_userid_len;
        cli_userid.value = cfg.br_userid;
      }
      else {
        cli_userid.length = cfg.dev_id_len;
        cli_userid.value = cfg.dev_id;
      }
      if( MQTT_SUCCESS != (rc = cli.set_br_userid( &cli, &cli_userid )) ) {
        TOLOG(LOG_ERR,"");
        goto restart;
      }
      /** Set User Name */
      if( cfg.br_username_len ) {
        cli_username.length = cfg.br_username_len;
        cli_username.value = cfg.br_username;
        if( MQTT_SUCCESS != (rc = cli.set_br_username( &cli, &cli_username )) ) {
          TOLOG(LOG_ERR,"");
          goto restart;   
        }
      }
      /** Set Password */
      if( cfg.br_pass_len ) {
        cli_password.length = cfg.br_pass_len;
        cli_password.value = cfg.br_pass;
        if( MQTT_SUCCESS != (rc = cli.set_br_password( &cli, &cli_password )) ) {
          TOLOG(LOG_ERR,"");
          goto restart;   
        }
      }
      mqtt_initialized = true;
      break;
    case SIG_TIMEOUT:
      data.length = 0;
    case SIG_RX:
      if(false == mqtt_initialized) {
        TOLOG(LOG_ERR, "");
        goto restart;        
      }
      /* Processing and sending */
      rc = cli.process( &cli, &data, NULL);
      if(rc != MQTT_SUCCESS && rc != MQTT_PENDING_DATA) {
        if(rc == MQTT_CONN_REJECTED) {
          TOLOG(LOG_ERR, "Connection rejected");
          system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0); 
        }
        else if(rc == MQTT_NOT_CONNECTED) {
          TOLOG(LOG_ERR, "Not connected");
          system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0);          
        }
        else {
          TOLOG(LOG_ERR, "");
        }
        break;
      }

      length = data.length;

      if( length && espconn ) {
        TOLOG(LOG_ERR, "Sending length: ");
        os_memset(tab, 0x00, sizeof(tab)/sizeof(tab[0]));
        os_sprintf(tab, "%d", length);
        TOLOG(LOG_ERR, tab);
        //for(i=0; i<length; ++i) {
        //  os_sprintf(tab, "%02x ", big_buffer[i]);
        //  TOLOG(LOG_INFO, tab);
        //}
        //TOLOG(LOG_INFO, "\r\n");
        if( 0 != (err=espconn_send( espconn, (uint8*) big_buffer, length)) ) {
          TOLOG(LOG_ERR, "");
        }
      }
      break;
    default:
      break;
  }

  if ( rc == MQTT_PENDING_DATA ) {
    /* new data could only be send if current data was sent successfully */
    pending_data = 1;
  }
  else {
    os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
    os_timer_arm(&mqtt_idle_timer, DELAY_1_SEC, 0);
  }

  return;

  restart:
    mqtt_restart_cb( NULL );
}