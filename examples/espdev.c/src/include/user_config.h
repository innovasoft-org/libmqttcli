#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/** Firmware code version
 * note: it is used during firmware update 
 * DO NOT MOVE LINE BELOW
 */
#define CODE_VERSION_VALUE      0x0008

#define DEV_TYPE_VALUE          "switch"

#define DEV_MODEL_VALUE         "s26"

/** Determines the size of buffer used to send / receive */
#define BUFFER_SIZE             1024

/** How many times station will try to get ip */
#define MAX_RETRY_CHECK_IP	    100

/** How many times station will send request to server */
#define MAX_RETRY_SEND_REQ      100

/** Delay time for network monitor timer function in ms */
#define DELAY_NET_MONITOR       100

/** Delay time for send function in ms */
#define DELAY_SEND_REQ_MONITOR  100

/** Time to force system restart */
#define DELAY_FORCE_RESTART     300000

/** Time to perform idle action */
#define DELAY_1_SEC             1000

/** Delay 5 seconds */
#define DELAY_5_SEC             5000

#define FUN_OK                  (uint16_t) (0x0000)
#define FUN_W_SENDING           (uint16_t) (0x0001)

#define FUN_E_UNKNOWN           (uint16_t) (0x8000)
#define FUN_E_ARGS              (uint16_t) (0x8001)
#define FUN_E_INTERNAL          (uint16_t) (0x8002)
#define FUN_E_VALUE             (uint16_t) (0x8003)
#define FUN_E_INV_USE           (uint16_t) (0x8004)

#define FUN_E_MEM_ALLOC         (uint16_t) (0x8101)
#define FUN_E_MEM_PERM          (uint16_t) (0x8102)
#define FUN_E_RFU               (uint16_t) (0xFFFF)

/** Available device modes */
enum device_modes {
  /** Undefined working mode */
  MODE_NUL = '\0',
  /** User working mode - allows to reconfigure the device (default mode) */
  MODE_CFG = 'C',
  /** Operational mode - device is working as mqtt server */
  MODE_OPE = 'O',
};

/** Port used with MQTT protocol */
#define MULTICAST_PORT          1025
#define MULTICAST_IP0           225
#define MULTICAST_IP1           0
#define MULTICAST_IP2           0
#define MULTICAST_IP3           1

/** Little Endian identifier */
#define LITTLE_ENDIAN 1
/** Big Endian identifier */
#define BIG_ENDIAN    2

/** Tensilica Xtensa L106 microprocessor is little endian */
#define BYTE_ORDER LITTLE_ENDIAN

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR              0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR            0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR    0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR              0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR            0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR    0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR               0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR              0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR            0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR    0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0x7c000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR               0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR              0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR            0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR    0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0xfc000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE                 0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR               0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR              0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR            0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR    0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR 0xfc000
#else
#error "The flash map is not supported"
#endif

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM      SYSTEM_PARTITION_CUSTOMER_BEGIN

#endif // __USER_CONFIG_H__
