#include <c_types.h>
#include <osapi.h>

#include "spi_flash.h"
#include "user_interface.h"
#include "user_cfg.h"
#include "user_log.h"
#include "user_util.h"

const uint8_t DEFAULT_DEV_NAME[] = "switch";
const uint8_t DEFAULT_DEV_SW[]   = "1.0";
const uint8_t DEFAULT_DEV_HW[]   = "1.0rev2";
const uint16_t DEFAULT_DEV_TTR   = 5000;

struct user_cfg cfg;

void cfg_set_defaults() {
  uint8 *addr = (uint8*) &cfg;
  uint32 chip_id;
  const char *hex = "0123456789ABCDEF";

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
  cfg.dev_name_len = ARRAYLEN(DEFAULT_DEV_NAME)-1;
  os_memcpy(cfg.dev_name, DEFAULT_DEV_NAME, cfg.dev_name_len );

  /* Parameter: dev_sw */
  cfg.dev_sw_len = ARRAYLEN(DEFAULT_DEV_SW)-1;
  os_memcpy(cfg.dev_sw, DEFAULT_DEV_SW, cfg.dev_sw_len );

  /* Parameter: dev_hw */
  cfg.dev_hw_len = ARRAYLEN(DEFAULT_DEV_HW)-1;
  os_memcpy(cfg.dev_hw, DEFAULT_DEV_HW, cfg.dev_hw_len );

  /* Parameter: dev_ttr */
  cfg.dev_ttr = DEFAULT_DEV_TTR;

  /* Parameter: dev_mode */
  cfg.dev_mode = MODE_CFG;
}

uint16_t cfg_init() {
  extern uint32 priv_param_start_sec;
  static uint8_t single_call = 0;
  struct rst_info *info;

  /* Force to call this function only once */
  if( 1 == single_call ) {
    return FUN_OK;
  }

  if( true != system_param_load(priv_param_start_sec, 0, &cfg, sizeof(struct user_cfg) )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  /* Check if defaults were loaded */
  if( MODE_CFG != cfg.dev_mode && MODE_OPE != cfg.dev_mode ) {
    cfg_set_defaults();
    /* Save data */
    if(FUN_OK != cfg_save()) {
      return FUN_E_INTERNAL;
    }
  }

  /* Set the function was called */
  single_call = 1;

  /* Load defaults */
  return FUN_OK;
}

uint16_t cfg_save() {
  extern uint32 priv_param_start_sec;

  /* Save parameters */
  if(true != system_param_save_with_protect(priv_param_start_sec, &cfg, sizeof(struct user_cfg) )) {
    TOLOG(LOG_ERR, "");
    return FUN_E_INTERNAL;
  }

  /* Success */
  return FUN_OK;
}
