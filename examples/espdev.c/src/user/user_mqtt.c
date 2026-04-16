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
/** Handler signal restart */
#define SIG_RESTART       8
/** Handler signal close */
#define SIG_CLOSE         10

const uint8_t GPIO0[] = "gpio0";
const uint8_t GPIO2[] = "gpio2";
const uint8_t GPIO4[] = "gpio4";
const uint8_t GPIO5[] = "gpio5";
const uint8_t GPIO12[] = "gpio12";
const uint8_t GPIO13[] = "gpio13";
const uint8_t GPIO14[] = "gpio14";
const uint8_t GPIO15[] = "gpio15";

#define STATE_DISCONNECTED    ( (uint8_t) 1 )
#define STATE_INITIALIZED     ( (uint8_t) 2 )
#define STATE_CONNECTED       ( (uint8_t) 3 )
#define STATE_AVAILABLE       ( (uint8_t) 4 )
#define STATE_UNSYNCHRONIZED  ( (uint8_t) 5 )
#define STATE_OPERATIONAL     ( (uint8_t) 6 )

/** Current value of idle timer */
static uint32_t timer_delay = DELAY_1_SEC;
/** Determines if there is pending data to send */
uint8_t pending_data;
/* Store device state */
uint8_t state = STATE_DISCONNECTED;
/* Stores idle counter */
uint32_t idle_counter;
/** Internal mqtt timer used to determine idle state */
os_timer_t mqtt_idle_timer;
/** Event queue */
os_event_t *mqtt_event_queue = NULL;
struct espconn *espconn;
extern const size_t big_buffer_len;
extern uint8_t big_buffer[1024];
extern uint8_t small_buffer[128];
clv_t data = { .capacity=sizeof(big_buffer)/sizeof(big_buffer[0]), .value=big_buffer };

static void ICACHE_FLASH_ATTR mqtt_handler(os_event_t *e);

void mqtt_delayed_reset() {
  /* Disarm idle timer */
  os_timer_disarm(&mqtt_idle_timer);
  /* clear the timer */
  timer_delay = 0;
  /* Send signal - RESTART */
  if( false == system_os_post(MQTT_HANDLER_ID, SIG_RESTART, 0)) {
    /* force to reconnect */
    net_connect(NULL);
  }
}

void ICACHE_FLASH_ATTR mqtt_reconnect_cb(void *arg)  {
  mqtt_reconnect_ex_cb(arg, 0);
}

void ICACHE_FLASH_ATTR mqtt_reconnect_ex_cb(void *arg, sint8 err)  {
  /* Disarm idle timer */
  os_timer_disarm(&mqtt_idle_timer);
  /* clear the timer */
  timer_delay = 0;
  /* Send signal - CLOSE */
  if( false == system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0)) {
    /* force to reconnect */
    net_connect(NULL);
  }
}

static void ICACHE_FLASH_ATTR mqtt_idle_cb(void *arg) {
  TOLOG(LOG_DEBUG, "idle");

  if(STATE_OPERATIONAL != state) {
    /* Switch on/off the led (there is negative polarization) */
    GPIO_OUTPUT_SET(13, !GPIO_INPUT_GET(13));
  }

  /* Send the event */
  if( false == system_os_post(MQTT_HANDLER_ID, SIG_TIMEOUT, 0)) {
    /* force to reconnect */
    net_connect(NULL);
  }
}

void ICACHE_FLASH_ATTR mqtt_ready_cb(void *arg) {
  extern struct user_cfg cfg;
  TOLOG(LOG_DEBUG, "mqtt ready");

  while( 1 ) {
    espconn = NULL;
    /* initialize task handler */
    if( NULL == (mqtt_event_queue = (os_event_t*) malloc( sizeof(os_event_t) * EVENT_QUEUE_LEN))) {
      break;
    }
    if( false == system_os_task(mqtt_handler, MQTT_HANDLER_ID, mqtt_event_queue, EVENT_QUEUE_LEN)) {
      break;
    }
    /* initialize timer and state  */
    timer_delay = DELAY_1_SEC;
    idle_counter = cfg.dev_ttr / timer_delay;
    state = STATE_DISCONNECTED;
    /* store current connection data */
    espconn = arg;
    /* send the event */
    if( false == system_os_post(MQTT_HANDLER_ID, SIG_INIT, 0) ) {
      break;
    }

    /* reset the timer for connection */
    net_reset_timer();

    /** success - return */
    return;
  }

  /* failure - force to connect */
  net_connect(NULL);
}

void ICACHE_FLASH_ATTR mqtt_udp_ready_cb() {
  TOLOG(LOG_DEBUG, "mqtt_udp_ready_cb");
}

void ICACHE_FLASH_ATTR mqtt_recv_cb(void *arg, char *pdata, unsigned short len) {
  uint8_t rc = 0;

  TOLOG(LOG_DEBUG, "mqtt_recv_cb");

  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  while(1) {
    if( big_buffer_len < len) {
      /* recv data too big */
      rc = 1;
      /* restart the timer */
      break;
    }
    if( 0 == len ) {
      /* send the event */
      if( false == system_os_post(MQTT_HANDLER_ID, SIG_CLOSE, 0)) {
        /* force to reconnect */
        net_connect(NULL);
      }
      /* connection was closed, do not run the timer */
      return;
    }
    if(data.length > 0 ) {
      // data in use
      rc = 2;
      /* restart the timer */
      break;
    }
    /* copy received data */
    os_memcpy(data.value, pdata, len);
    data.length = len;
    os_sprintf(small_buffer, "recv len = %d", data.length);
    TOLOG(LOG_DEBUG, small_buffer);
    // size_t i;
    // for(i=0; i<data.length; ++i) {
    //  os_sprintf(tab, "%02x ", big_buffer[i]);
    //  TOLOG(LOG_INFO, tab);
    // }
    // TOLOG(LOG_INFO, "\r\n");

    /* send the event */
    if( false == system_os_post(MQTT_HANDLER_ID, SIG_RX, 0)) {
      /* force to reconnect */
      net_connect(NULL);
    }
    /* do not start the timer, to not disturb */
    return;
  }

  /* failure - print info */
  os_sprintf(small_buffer, "rc = %d", rc);
  TOLOG(LOG_DEBUG, small_buffer);

  if(timer_delay) {
    /* start idle timer */
    os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
    os_timer_arm(&mqtt_idle_timer, timer_delay, 0);
  }
}

void ICACHE_FLASH_ATTR mqtt_udp_recv_cb(void *arg, char *pdata, unsigned short len) {
  remot_info *premote = NULL;
  ip_addr_t ip;
  uint8_t ins, rc = 0;

  TOLOG(LOG_DEBUG, "udp recv");

  /* Disarm timers */
  os_timer_disarm(&mqtt_idle_timer);

  while(1) {
    if( !arg || 2 != len || 0x00 != pdata[1]) {
      /* restart the timer */
      rc = 1;
      break;
    }

    ins = pdata[0];

    if(0xf0 == ins) {
      /* Send only ping response */
      pdata[0] = 0xff;
      if( FUN_OK != net_udp_sendto(pdata, len, net_ip_cast(arg), MULTICAST_PORT)) {
        rc = 2;
        break;
      }
    }
    else if(0xf1 == ins) {
      if(state < STATE_CONNECTED) {
        /* force connect to the broker */
        if( ESPCONN_OK != espconn_get_connection_info((struct espconn*) arg, &premote, 0) ) {
          TOLOG(LOG_ERR, "");
          net_connect( NULL );
          return;
        }
        ip.addr = premote->remote_ip[0] | (premote->remote_ip[1]<<8) | (premote->remote_ip[2]<<16) | (premote->remote_ip[3]<<24);
        net_connect( &ip );
        return;
      }
    }
    else if(0xf2 == ins) {
      /* reset device */
      wifi_station_disconnect();
      system_restart();
      return;
    }
    else if(0xf3 == ins) {
      /* restore factory settings */
      cfg_set_defaults();
      cfg_save();
      system_restart();
      return;
    }

    /* restart the timer */
    break;
  }

  /* failure - print info */
  os_sprintf(small_buffer, "rc = %d", rc);
  TOLOG(LOG_DEBUG, small_buffer);

  if(timer_delay) {
    /* start idle timer */
    os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
    os_timer_arm(&mqtt_idle_timer, timer_delay, 0);
  }
}

void ICACHE_FLASH_ATTR mqtt_sent_cb(void *arg) {
  TOLOG(LOG_DEBUG, "sent");
  data.length = 0;
  if( pending_data ) {
    pending_data = 0;
    if( false == system_os_post(MQTT_HANDLER_ID, SIG_TIMEOUT, 0)) {
      /* force to reconnect */
      net_connect(NULL);
    }
  }
}

void ICACHE_FLASH_ATTR mqtt_udp_sent_cb(void *arg) {
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
  uint8_t *ptr = NULL;
  mqtt_subscribe_params_t subscribe_params = { };

  if( RC_SUCCESS != pkt->rc) {
    TOLOG(LOG_ERR, "");
    /* will be reconnected */
    mqtt_reconnect_cb( NULL );
    return RC_SUCCESS;
  }

  /* Subscribing to receive Home Assistant status (it shall be configured in mosquito plugin) */
  ptr = data.value;
  subscribe_params.filter.value = ptr;
  subscribe_params.filter.length = os_sprintf( ptr, "%s/%s", cfg.ha_base_t, cfg.ha_birth_t );
  TOLOG(LOG_INFO, ptr);
  if(MQTT_SUCCESS != self->subscribe(self, &subscribe_params)) {
    TOLOG(LOG_ERR, "");
    /* will be restarted */
    mqtt_delayed_reset();
    return RC_SUCCESS;
  }
  timer_delay = DELAY_1_SEC;
  idle_counter = cfg.dev_ttr / timer_delay;
  state = STATE_CONNECTED;

  return RC_SUCCESS;
}

mqtt_rc_t cb_publish(const mqtt_cli_ctx_cb_t *self, const mqtt_publish_t *pkt, const mqtt_channel_t *channel) {
  extern struct user_cfg cfg;
  extern volatile int gpio_num;
  int gpio_state, c1, c2;
  mqtt_publish_params_t publish_params = { };
  mqtt_subscribe_params_t subscribe_params = { };
  size_t offset, topic_len;
  uint8_t *ptr, *message, *topic;
  uint32_t version;

  /* Check if Home Assistant is online */
  topic = &small_buffer[0];
  topic_len = os_sprintf( topic, "%s/%s", cfg.ha_base_t, cfg.ha_birth_t );
  if( (topic_len == pkt->topic.length) && 
      (0 == os_memcmp(topic, pkt->topic.value, pkt->topic.length)) &&
      (cfg.ha_pl_avail_len == pkt->message.length) &&
      (0 == os_memcmp(cfg.ha_pl_avail, pkt->message.value, pkt->message.length)) ) {
    /* Publishing configuration */
    mqtt_cli_get_lib_version( &version );
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
    offset += os_sprintf( message + offset, "\"o\": {\"name\":\"mqttcli\",\"sw\": \"%08x\",\"url\": \"https://www.innovasoft.org\"}", version);
    offset += os_sprintf( message + offset, "}");
    publish_params.message.length = offset;
    /* message will be retained */
    publish_params.flags = 0x01;
    if( MQTT_SUCCESS != self->publish(self, &publish_params) ) {
      TOLOG(LOG_ERR,"");
      /* will be restarted */
      mqtt_delayed_reset();
      return RC_IMPL_SPEC_ERR;
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
      TOLOG(LOG_ERR,"");
      /* will be restarted */
      mqtt_delayed_reset();
      return RC_IMPL_SPEC_ERR;
    }

    timer_delay = DELAY_1_SEC;
    // previous value was 10  
    idle_counter = 1 + cfg.dev_ttc / timer_delay;
    state = STATE_AVAILABLE;
    return RC_SUCCESS;
  }

  /* Check if Home Assistant is offline */
  topic = &small_buffer[0];
  topic_len = os_sprintf( topic, "%s/%s", cfg.ha_base_t, cfg.ha_will_t );
  if( (topic_len == pkt->topic.length) && 
      (0 == os_memcmp(topic, pkt->topic.value, pkt->topic.length)) &&
      (cfg.ha_pl_not_avail_len == pkt->message.length) &&
      (0 == os_memcmp(cfg.ha_pl_not_avail, pkt->message.value, pkt->message.length)) ) {
    TOLOG(LOG_ERR,"");
    /* will be reconnected */
    mqtt_reconnect_cb( NULL );
    return RC_SUCCESS;
  }

  if(STATE_OPERATIONAL != state) {
    TOLOG(LOG_ERR,"");
    return RC_IMPL_SPEC_ERR;
  }

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
  /* message will be retained */
  publish_params.flags = 0x01;
  if(MQTT_SUCCESS != self->publish(self, &publish_params)) {
    TOLOG(LOG_ERR,"");
    /* will be restarted */
    mqtt_delayed_reset();
    /* connection will be restarted */
    return RC_IMPL_SPEC_ERR;
  }

  /** Save current GPIO state in RTC memory (RAM), shall be last line !!! */
  gpio_num = (1 == gpio_state) ? gpio_num : 0;
  system_rtc_mem_write(64, (void*) &gpio_num, sizeof(gpio_num));

  return RC_SUCCESS;
}

static void ICACHE_FLASH_ATTR mqtt_handler(os_event_t *e) {
  extern struct user_cfg cfg;
  static mqtt_cli_t cli;
  uint16_t rc = MQTT_SUCCESS;
  mqtt_params_t mqtt_params = { .bufsize=big_buffer_len, .max_pkt_id=8, .timeout=1, .version=4 };
  mqtt_will_params_t will_params = (mqtt_will_params_t) { };
  lv_t cli_userid, cli_username, cli_password;
  uint8_t *ptr = NULL, *message;
  size_t length;
  mqtt_publish_params_t publish_params = { };
  int gpio_state;
  extern volatile int gpio_num;

  TOLOG(LOG_DEBUG, "mqtt_handler");

  /* Disarm idle timer */
  os_timer_disarm(&mqtt_idle_timer);

  if(NULL == e) {
    /* force hard reset */
    system_restart();
    return;
  }

  switch(e->sig) {
    case SIG_CLOSE:
      TOLOG(LOG_CRIT,"SIG_CLOSE");
      if(state != STATE_DISCONNECTED) {
        mqtt_cli_destr( &cli );
        state = STATE_DISCONNECTED;
        timer_delay = idle_counter = 0;
        net_connect( NULL );
      }
      break;
    case SIG_RESTART:
      TOLOG(LOG_CRIT,"SIG_RESTART");
      if(state != STATE_DISCONNECTED) {
        mqtt_cli_destr( &cli );
        state = STATE_DISCONNECTED;
      }
      /* initiate delayed restart */
      timer_delay = DELAY_500_MS;
      idle_counter = cfg.dev_ttr / timer_delay;
      /* to give a chance for the user to restore factory settings */
      idle_counter *= 60;
      break;
    case SIG_INIT:
      state = STATE_DISCONNECTED;      
      if( MQTT_SUCCESS != (rc = mqtt_cli_init_ex( &cli, &mqtt_params )) ) {
        /* Protocol initialization has failed */
        TOLOG(LOG_CRIT,"");
        /* connection will be restarted */
        mqtt_delayed_reset();
        break;
      }
      cli.set_cb_connack( &cli, cb_connack );
      cli.set_cb_publish( &cli, cb_publish );
      cli.set_br_keepalive( &cli, (uint16_t) 60);
      /** Set the Will */
      ptr = data.value;
      will_params.topic.value = ptr;
      if(cfg.ha_node_id_len) {
        will_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.ha_node_id, cfg.dev_id, cfg.ha_avty_t);
      }
      else {
        will_params.topic.length = os_sprintf( ptr, "%s/%s/%s/%s", cfg.ha_base_t, cfg.dev_name, cfg.dev_id, cfg.ha_avty_t);
      }
      message = will_params.payload.value = ptr + will_params.topic.length;
      will_params.payload.length = os_sprintf( message, "%s", cfg.ha_pl_not_avail );
      /* WILL will be retained */
      will_params.retain = 1;
      if( MQTT_SUCCESS != (rc = cli.set_br_will( &cli, &will_params) ) ) {
        /* Protocol WILL configuration failed */
        TOLOG(LOG_ERR,"");
        /* connection will be restarted */
        mqtt_delayed_reset();
        break;
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
        /* Protocol user id configuration failed */
        TOLOG(LOG_ERR,"");
        /* connection will be restarted */
        mqtt_delayed_reset();
        break;
      }
      /** Set User Name */
      if( cfg.br_username_len ) {
        cli_username.length = cfg.br_username_len;
        cli_username.value = cfg.br_username;
        if( MQTT_SUCCESS != (rc = cli.set_br_username( &cli, &cli_username )) ) {
          /* Protocol user name configuration failed */
          TOLOG(LOG_ERR,"");
          /* connection will be restarted */
          mqtt_delayed_reset();
          break;
        }
      }
      /** Set Password */
      if( cfg.br_pass_len ) {
        cli_password.length = cfg.br_pass_len;
        cli_password.value = cfg.br_pass;
        if( MQTT_SUCCESS != (rc = cli.set_br_password( &cli, &cli_password )) ) {
          /* Protocol password configuration failed */
          TOLOG(LOG_ERR,"");
          /* connection will be restarted */
          mqtt_delayed_reset();
          break;
        }
      }
      state = STATE_INITIALIZED;
      break;
    case SIG_TIMEOUT:
      if( (state == STATE_CONNECTED || state == STATE_INITIALIZED || state == STATE_DISCONNECTED || state == STATE_AVAILABLE || state == STATE_UNSYNCHRONIZED) && idle_counter > 0 ) {
        --idle_counter;
      }
      else if (state == STATE_AVAILABLE) {
        state = STATE_UNSYNCHRONIZED;
        timer_delay = DELAY_1_SEC;
        idle_counter = cfg.dev_ttr / timer_delay;
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
        /* message will be retained */
        publish_params.flags = 0x01;
        if(MQTT_SUCCESS != cli.publish( &cli, &publish_params)) {
          /* Publishing has failed */
          TOLOG(LOG_ERR,"");
          /* connection will be restarted */
          mqtt_delayed_reset();
          break;
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
        /* message will be retained */
        publish_params.flags = 0x01;
        if(MQTT_SUCCESS != cli.publish(&cli, &publish_params)) {
          /* Publishing has failed */
          TOLOG(LOG_ERR,"");
          /* connection will be restarted */
          mqtt_delayed_reset();
          break;
        }
        /* Switch off the led (there is negative polarization) */
        GPIO_OUTPUT_SET(13, 1);
        timer_delay = DELAY_1_SEC;
        idle_counter = 0;
        state = STATE_OPERATIONAL;
      }
      else if (state == STATE_DISCONNECTED || state == STATE_INITIALIZED || state == STATE_CONNECTED || state == STATE_UNSYNCHRONIZED ) {
        /* Restart the system */
        system_restart();
        return;
      }
      data.length = 0;
    case SIG_RX:
      if(STATE_DISCONNECTED == state) {
        break;
      }
      /* Processing and sending */
      rc = cli.process( &cli, &data, NULL);
      length = data.length;
      if( rc == MQTT_PENDING_DATA ) {
        /* new data could only be send if current data was sent successfully */
        pending_data = 1;
      }
      else if(rc != MQTT_SUCCESS) {
        if(rc == MQTT_CONN_REJECTED) {
          /* Connection rejected */
          TOLOG(LOG_ERR, cfg.br_host);
          TOLOG(LOG_ERR, cfg.br_username);
          TOLOG(LOG_ERR, cfg.br_pass);
          /* connection will be restarted */
          mqtt_delayed_reset();
          break;
        }
        else if(rc == MQTT_NOT_CONNECTED) {
          /* Not connected */
          TOLOG(LOG_ERR, "");
          mqtt_reconnect_cb( NULL );
          break;
        }
        else {
          TOLOG(LOG_ERR, "");
        }
        break;
      }

      if( length && espconn ) {
        os_sprintf(small_buffer, "send len = %d", length);
        TOLOG(LOG_DEBUG, small_buffer);
        // size_t i;
        // for(i=0; i<length; ++i) {
        //  os_sprintf(tab, "%02x ", big_buffer[i]);
        //  TOLOG(LOG_INFO, tab);
        // }
        // TOLOG(LOG_INFO, "\r\n");
        if( 0 != espconn_send( espconn, (uint8*) big_buffer, length) ) {
          TOLOG(LOG_ERR, "");
          mqtt_reconnect_cb( NULL );
          break;
        }
      }
      break;
    default:
      break;
  }

  if(timer_delay) {
    /* start idle timer */
    os_timer_setfn(&mqtt_idle_timer, (os_timer_func_t *)mqtt_idle_cb, NULL);
    os_timer_arm(&mqtt_idle_timer, timer_delay, 0);
  }
}