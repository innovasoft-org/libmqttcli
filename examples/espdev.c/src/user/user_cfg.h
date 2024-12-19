#ifndef __USER_CFG_H__
#define __USER_CFG_H__



/** Program configuration structure */
struct user_cfg {
  /** Device mode */
  uint8_t dev_mode;               /*  1 */
  /** Device name e.g: switch */
  uint8_t dev_name[32];           /* 32 */
  /** Device name length */
  uint8_t dev_name_len;           /*  1 */
  /** Device id */
  uint8_t dev_id[16];             /* 16 */
  /** Device ID length */
  uint8_t dev_id_len;             /*  1 */
  /* Device software version */
  uint8_t dev_sw[16];             /* 16 */
  /* Device software version length */
  uint8_t dev_sw_len;             /*  1 */
  /* Device hardware version */
  uint8_t dev_hw[16];             /* 16 */
  /* Device hardware version length */
  uint8_t dev_hw_len;             /*  1 */
  /** Wi-Fi ssid */
  uint8_t wifi_ssid[64];          /* 64 */
  /** Wi-Fi ssid length */
  uint8_t wifi_ssid_len;          /*  1 */
  /** Wi-Fi password */
  uint8_t wifi_pass[64];          /* 64 */
  /** Wi-Fi password length */
  uint8_t wifi_pass_len;          /*  1 */
  /** Access Point password */
  uint8_t ap_pass[64];            /* 64 */
  /** Access Point password length */
  uint8_t ap_pass_len;            /*  1 */
  /** Broker's host name */
  uint8_t br_host[64];            /* 64 */
  /** Broker's host name length */
  uint8_t br_host_len;            /*  1 */
  /** Broker's port number */
  uint32_t br_port;               /*  4 */
  /** Broker's user ID */
  uint8_t br_userid[64];          /* 64 */
  /** Broker's user ID length */
  uint8_t br_userid_len;          /*  1 */
  /** Broker's user name */
  uint8_t br_username[64];        /* 64 */
  /** Broker's user name length */
  uint8_t br_username_len;        /*  1 */
  /** Broker's user password */
  uint8_t br_pass[64];            /* 64 */
  /** Broker's user password length */
  uint8_t br_pass_len;            /*  1 */
  /** HA's base topic */
  uint8_t ha_base_t[64];          /* 64 */
  /** HA's base topic length */
  uint8_t ha_base_t_len;          /*  1 */
  /** Node id - eg. gpio12 */
  uint8_t ha_node_id[32];         /* 32 */
  /** Device node length */
  uint8_t ha_node_id_len;         /*  1 */
  /** HA's command topic */
  uint8_t ha_cmd_t[32];           /* 32 */
  /** HA's command topic length */
  uint8_t ha_cmd_t_len;           /*  1 */
  /** HA's state topic */
  uint8_t ha_stat_t[32];          /* 32 */
  /** HA's state topic length */
  uint8_t ha_stat_t_len;          /*  1 */
  /** HA's availability topic */
  uint8_t ha_avty_t[32];          /* 32 */
  /** HA's availability topic length */
  uint8_t ha_avty_t_len;          /*  1 */
  /** HA's payload on */
  uint8_t ha_pl_on[16];           /* 16 */
  /** HA's payload on length */
  uint8_t ha_pl_on_len;           /*  1 */
  /** HA's payload off */
  uint8_t ha_pl_off[16];          /* 16 */
  /** HA's payload off length */
  uint8_t ha_pl_off_len;          /*  1 */
  /** HA's payload available */
  uint8_t ha_pl_avail[16];        /* 16 */
  /** HA's payload available length */
  uint8_t ha_pl_avail_len;        /*  1 */
  /** HA's payload not available */
  uint8_t ha_pl_not_avail[16];    /* 16 */
  /** HA's payload not available length */
  uint8_t ha_pl_not_avail_len;    /*  1 */
  /** HA's state on */
  uint8_t ha_stat_on[16];         /* 16 */
  /** HA's state on length */
  uint8_t ha_stat_on_len;         /*  1 */
  /** HA's state off */
  uint8_t ha_stat_off[16];        /* 16 */
  /** HA's state off length */
  uint8_t ha_stat_off_len;        /*  1 */
  /** Used to make total structure size dividable by 4 */
  uint8_t rfu[13];                /* 13 */
};

uint16_t cfg_init();

void cfg_set_defaults( void );

uint16_t cfg_save( void );

#endif /* __USER_SPI_H__ */