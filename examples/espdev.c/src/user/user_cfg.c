#include <c_types.h>
#include <osapi.h>

#include "user_cfg.h"
#include "user_log.h"
#include "spi_flash.h"
#include "user_interface.h"

const uint8_t DEV_NAME[] = "switch";
const uint8_t DEV_SW[]   = "1.0";
const uint8_t DEV_HW[]   = "1.0rev2";

struct user_cfg cfg;
static uint8_t cfg_initialized = false;

void cfg_set_defaults() {
  uint8 *addr = (uint8*) &cfg;
  uint32 chip_id;
  const char *hex = "0123456789ABCDEF";

  TOLOG(LOG_DEBUG, "cfg_set_defaults()\r\n");

  memset(addr, 0, sizeof(struct user_cfg));

  /* Parameter: dev_id */
  chip_id = system_get_chip_id();
  cfg.dev_id[ 0] = 'E';
  cfg.dev_id[ 1] = 'S';
  cfg.dev_id[ 2] = 'P';
  cfg.dev_id[ 3] = '-';
  cfg.dev_id[ 4] = hex[(((uint8) (chip_id >> 24))>>4)&0xF];
  cfg.dev_id[ 5] = hex[(((uint8) (chip_id >> 24))   )&0xF];
  cfg.dev_id[ 6] = hex[(((uint8) (chip_id >> 16))>>4)&0xF];
  cfg.dev_id[ 7] = hex[(((uint8) (chip_id >> 16))   )&0xF];
  cfg.dev_id[ 8] = hex[(((uint8) (chip_id >>  8))>>4)&0xF];
  cfg.dev_id[ 9] = hex[(((uint8) (chip_id >>  8))   )&0xF];
  cfg.dev_id[10] = hex[(((uint8) (chip_id      ))>>4)&0xF];
  cfg.dev_id[11] = hex[(((uint8) (chip_id      ))   )&0xF];
  cfg.dev_id_len = 12;

  /* Parameter: dev_name */
  cfg.dev_name_len = ARRAYLEN(DEV_NAME)-1;
  os_memcpy(cfg.dev_name, DEV_NAME, cfg.dev_name_len );

  /* Parameter: dev_sw */
  cfg.dev_sw_len = ARRAYLEN(DEV_SW)-1;
  os_memcpy(cfg.dev_sw, DEV_SW, cfg.dev_sw_len );

  /* Parameter: dev_hw */
  cfg.dev_hw_len = ARRAYLEN(DEV_HW)-1;
  os_memcpy(cfg.dev_hw, DEV_HW, cfg.dev_hw_len );

  /* Parameter: dev_mode */
  cfg.dev_mode = MODE_CFG;
}

uint16_t cfg_init() {
  extern uint32 priv_param_start_sec;
  static uint8_t single_call = 0;
  struct rst_info *info;
  
  TOLOG(LOG_DEBUG, "cfg_init()\r\n");

  /* Force to call this function only once */
  if( 1 == single_call ) {
    return FUN_OK;
  }
  /* Set the function was called */
  single_call = 1;

  if( true != system_param_load(priv_param_start_sec, 0, &cfg, sizeof(struct user_cfg) )) {
    TOLOG(LOG_ERR, "system_param_load() failed\r\n");
    return FUN_E_INTERNAL;
  }

  /* Check if defaults were loaded */
  if( MODE_CFG != cfg.dev_mode && MODE_OPE != cfg.dev_mode ) {
    cfg_set_defaults();
    if(FUN_OK != cfg_save()) {
      return FUN_E_INTERNAL;
    }
  }

  /* Set the cfg was initialized */
  cfg_initialized = true;

  /* Load defaults */
  return FUN_OK;
}

uint16_t cfg_save( void ) {
  extern uint32 priv_param_start_sec;

  TOLOG(LOG_DEBUG, "cfg_save()\r\n");

  /* If configuration was not (initialized) loaded */
  if( false == cfg_initialized ) {
    TOLOG(LOG_ERR, "Uninitialized\r\n");
    return FUN_E_INV_USE;
  }

  if(true != system_param_save_with_protect(priv_param_start_sec, &cfg, sizeof(struct user_cfg) )) {
    TOLOG(LOG_ERR, "system_param_save_with_protect() failed\r\n");
    return FUN_E_INTERNAL;
  }

  return FUN_OK;
}
