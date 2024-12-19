#ifndef __COMMON_H__
#define __COMMON_H__

#include "../../api/mqtt_common.h"

/** Program name */
#define PROGRAM_NAME      "mqtt"
/** Program's author */
#define PROGRAM_AUTHOR    "Jakub Piwowarczyk"
/** Program's version*/
#define PROGRAM_VERSION   "1.0.0.0"

#define RESULT_OK         (int) (0)
#define RESULT_FAILURE    (int) (65535)
#define RESULT_EXIT       (int) (5)

#define DEFAULT_IP    "127.0.0.1"
#define DEFAULT_PORT  1884
#define DEFAULT_BUFFER_SIZE 1024

/* Short option: port */
#define S_OPT_PORT          'p'
/* Long option: port */
#define L_OPT_PORT          "port"
/* Short option: group */
#define S_OPT_HOST          'h'
/* Long option: group */
#define L_OPT_HOST          "host"
/** Short option: buffer size*/
#define S_OPT_BUFFER_SIZE   'b'
/** Long option: buffer size */
#define L_OPT_BUFFER_SIZE   "buffer-size"
/** Short option: reuse address */
#define S_OPT_REUSE_ADDR    'r'
/** Long option: reuse address */
#define L_OPT_REUSE_ADDR    "reuse-addr"
/** Short option: topic */
#define S_OPT_TOPIC         't'
/** Long option: topic */
#define L_OPT_TOPIC         "topic"
/** Short option: message */
#define S_OPT_MESSAGE       'm'
/** Long option: message */
#define L_OPT_MESSAGE       "message"
/** Short option: user id */
#define S_OPT_USERID        'I'
/** Long option: user id */
#define L_OPT_USERID        "userid"
/** Short option: user name */
#define S_OPT_USERNAME      'N'
/** Long option: user name */
#define L_OPT_USERNAME       "username"
/** Short option: password */
#define S_OPT_PASSWORD      'P'
/** Long option: password */
#define L_OPT_PASSWORD      "password"
/** Short option: verbose */
#define S_OPT_VERBOSE      'v'
/** Long option: verbose */
#define L_OPT_VERBOSE       "verbose"
/** Short option: publish */
#define S_OPT_PUBLISH       '\1'
/** Long option: publish */
#define L_OPT_PUBLISH       "publish"
/** Long option: pub */
#define L_OPT_PUB           "pub"
/** Short option: subscribe */
#define S_OPT_SUBSCRIBE     '\2'
/** Long option: subscribe */
#define L_OPT_SUBSCRIBE     "subscribe"
/** Long option: sub */
#define L_OPT_SUB           "sub"
/** Short option: mqtt-version */
#define S_OPT_MQTT_VERSION  '\3'
/** Long option: mqtt-version */
#define L_OPT_MQTT_VERSION  "mqtt-version"
/** Short option: cafile */
#define S_OPT_CAFILE        '\4'
/** Long option: cafile */
#define L_OPT_CAFILE        "cafile"
/** Short option: capath */
#define S_OPT_CAPATH        '\5'
/** Long option: capath */
#define L_OPT_CAPATH        "capath"
/** Short option: cert */
#define S_OPT_CERT          '\6'
/** Long option: cert */
#define L_OPT_CERT          "cert"
/** Short option: key */
#define S_OPT_KEY           '\7'
/** Long option: key */
#define L_OPT_KEY           "key"

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

/** @brief Program context definition */
typedef struct program_ctx {
  /** IP to bind to */
  char ip[128];
  /** Port number to use */
  int port;
  char port_str[10];
  /* Option reuse addr */
  int optval_reuse_addr;
  /* Non blocking */
  int non_blocking;
  /** Buffer size */
  int buffer_size;
  /** Time stamps */
  int time_stamps;
  /** Non blocking mode timeout */
  int timeout;
  /** Subscribe */
  int subscribe;
  /** Publish */
  int publish;
  /** MQTT topic */
  char topic[MAX_TOPIC_LEN+1];
  /** MQTT message */
  char message[MAX_MESSAGE_LEN+1];
  /** Log maximum level which will be printed */
  int log_max_level;
  /** Log file descriptor (determines were to log the data) */
  FILE *log_fd;
  /** User ID */
  char userid[MAX_USERID_LEN+1];
  /** User Name */
  char username[MAX_USERNAME_LEN+1];
  /** User Name */
  char password[MAX_PASSWORD_LEN+1];
  /** Verbose */
  uint8_t verbose;
  /** Stores program state */
  uint8_t state;
  /** Stores timer interrupt status */
  uint8_t timer_int;
  /** Stores MQTT protocol's version */
  uint8_t mqtt_version;
  /** Stores 1 if TLS is enabled, otherwise 0 */
  uint8_t tls;
  /** Stores path to the CA certificate */
  char *cafile;
  /** Stores path to certificates used during chain verification */
  char *capath;
  /** Stores the path to the user's certificate */
  char *cert;
  /** Stores the path to the user's certificate private key*/
  char* key;
} context_t;

#define IS_MULTICAST(IPADDR) ( (IPADDR & 0x000000E0) == 0x000000E0 )

void log_write(int level, char* filename, int line, char *fmt,...);

#define TOLOG(level, ...) log_write(level, __FILE__, __LINE__, __VA_ARGS__ )

#endif /* __COMMON_H__ */