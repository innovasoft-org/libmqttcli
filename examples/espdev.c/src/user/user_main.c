#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"

#include "user_log.h"
#include "user_net.h"
#include "user_server.h"
#include "user_mqtt.h"
#include "user_cfg.h"

uint32 priv_param_start_sec;
uint8_t big_buffer[1024] = { 0 };
const size_t big_buffer_len = sizeof(big_buffer) / sizeof(uint8_t);

/** Button pressed counter */
volatile int button_counter = 0;

volatile int gpio_num = 12;

/** Button pressed monitoring timer */
static os_timer_t button_monitor_timer;

/** GPIO interrupt callback */
static void gpio_intr_cb(void *param);

/** Callback for button pressed timer */
static void ICACHE_FLASH_ATTR button_monitor_cb(void *arg);

static const partition_item_t at_partition_table[] = {
  { SYSTEM_PARTITION_BOOTLOADER, 0x0, 0x1000},
  { SYSTEM_PARTITION_OTA_1, 0x1000, SYSTEM_PARTITION_OTA_SIZE},
  { SYSTEM_PARTITION_OTA_2, SYSTEM_PARTITION_OTA_2_ADDR, SYSTEM_PARTITION_OTA_SIZE},
  { SYSTEM_PARTITION_RF_CAL, SYSTEM_PARTITION_RF_CAL_ADDR, 0x1000},
  { SYSTEM_PARTITION_PHY_DATA, SYSTEM_PARTITION_PHY_DATA_ADDR, 0x1000},
  { SYSTEM_PARTITION_SYSTEM_PARAMETER, SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 0x3000},
  { SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM, SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR, 0x1000},
};

void ICACHE_FLASH_ATTR user_pre_init(void) {
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
      os_printf("system_partition_table_regist fail\r\n");
      while(1);
    }
}

/* It should be added with the SDK 2.0.0.0 */
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
  enum flash_size_map size_map = system_get_flash_size_map();
  uint32 rf_cal_sec = 0;

  switch (size_map) {
    case FLASH_SIZE_4M_MAP_256_256:
      rf_cal_sec = 128 - 8;
      break;
    case FLASH_SIZE_8M_MAP_512_512:
      rf_cal_sec = 256 - 5;
      break;
    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
      rf_cal_sec = 512 - 5;
      break;
    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
      rf_cal_sec = 1024 - 5;
      break;
    default:
      rf_cal_sec = 0;
      break;
  }

  return rf_cal_sec;
}

static void ICACHE_FLASH_ATTR button_monitor_cb(void *arg) {
  extern struct user_cfg cfg;

  TOLOG(LOG_DEBUG, "button_monitor_cb()");

  os_timer_disarm(&button_monitor_timer);

  /* if GPIO0 is still pressed */
  if( 0 == GPIO_INPUT_GET( 0 ) ) {
    ++button_counter;
    /* trigger new timer */
    os_timer_setfn(&button_monitor_timer, (os_timer_func_t *)button_monitor_cb, NULL);
    os_timer_arm(&button_monitor_timer, DELAY_1_SEC, 0);
  }
  else {
    /* check the counter */
    if (button_counter >= 5 && button_counter < 10) {
      TOLOG(LOG_INFO, "Restarting device...");
      wifi_station_disconnect();
      system_restart();
    }
    else if (button_counter >= 10 ) {
      TOLOG(LOG_INFO, "Restoring factory settings...");
      cfg_set_defaults();
      cfg_save();
      system_restart();
    }
    button_counter = 0;
  }
}

static void  gpio_intr_cb(void *param) {
  /* clear gpio status. See SDK Programming Guide in  5.1.6. GPIO interrupt handler */
  uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

  /* if the interrupt was by GPIO0 */
  if (gpio_status & BIT(0)) {
    /* disable interrupt for GPIO0 */
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);

    /* if GPIO0 was pressed */
    if( 0 == GPIO_INPUT_GET( 0 ) ) {
      /* increment button counter */
      TOLOG(LOG_DEBUG, "Button pressed...");
      /* start button timer */
      os_timer_disarm(&button_monitor_timer);
      os_timer_setfn(&button_monitor_timer, (os_timer_func_t *)button_monitor_cb, NULL);
      os_timer_arm(&button_monitor_timer, DELAY_1_SEC, 0);
    }

    /* clear interrupt status for GPIO0 */
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(0));

    /* Reactivate interrupts for GPIO0 */
    gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_ANYEDGE);
  }
}

void ICACHE_FLASH_ATTR system_run_cb() {
  extern struct user_cfg cfg;
  int i;

  /* Configure GPIO pins */
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);

  /* Switch on the led - device was run */
  GPIO_OUTPUT_SET(13, 0);

  /* Read system configuration */
  cfg_init();

  /* Configure internal pull up for GPIO0 */
  PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);

  /* Set GPIO0 as input */
  gpio_output_set(0, 0, 0, GPIO_ID_PIN(0));

  /* Disable interrupts for GPIO */
  ETS_GPIO_INTR_DISABLE();

  /* Attach interrupt handle to gpio interrupts */
  ETS_GPIO_INTR_ATTACH(gpio_intr_cb, &button_counter);

  /* Configure GPIO0 */
  gpio_register_set (GPIO_PIN_ADDR(0),
                    GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)  |
                    GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE) |
                    GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

  /* Clear gpio status. Say ESP8266EX SDK Programming Guide in  5.1.6. GPIO interrupt handler */
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));

  /* Configure interrupt for GPIO0 */
  gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_NEGEDGE);

  /* Enable interrupts for GPIO */
  ETS_GPIO_INTR_ENABLE();
  
  /* Start application mode */
  switch( cfg.dev_mode ) {
    case MODE_CFG:
      /* Initializing callbacks */
      net_tcp_regist_recv_cb( server_recv_cb );
      net_tcp_regist_sent_cb( server_sent_cb );
      /* Initialize network connection (it should be the last) */
      net_init( SOFTAP_MODE, ESPCONN_TCP );
      break;
    case MODE_OPE:
      /* Initializing callbacks */
      net_regist_wifi_disconnected_cb( mqtt_restart_cb );
      net_tcp_regist_recv_cb( mqtt_recv_cb );
      net_tcp_regist_sent_cb( mqtt_sent_cb );
      net_tcp_regist_connect_cb( mqtt_ready_cb );
      net_tcp_regist_recon_cb( matt_reconnect_cb );
      net_tcp_regist_discon_cb( mqtt_disconnect_cb );
      net_udp_regist_recv_cb( mqtt_udp_recv_cb );
      net_udp_regist_ready_cb( mqtt_udp_ready_cb );
      /* Initialize network connection (it should be the last) */
      net_init( STATION_MODE, ESPCONN_TCP | ESPCONN_UDP );
      break;
    default:
      TOLOG(LOG_ALERT, "");
      wifi_station_disconnect();
      system_restart();
      return;
  }
}

// Init function
void ICACHE_FLASH_ATTR user_init() {
  partition_item_t partition_item;

  /* Initialize logging (it should be the first) */
  log_init(LOG_DEBUG);

  /* Initialize GPIO */
  gpio_init();

  /* Calculate user data start sector */
  if (!system_partition_get_item(SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM, &partition_item)) {
    TOLOG(LOG_CRIT, "Get partition information fail");
  }
  priv_param_start_sec = partition_item.addr / SPI_FLASH_SEC_SIZE;
  
  system_init_done_cb( system_run_cb );
}
