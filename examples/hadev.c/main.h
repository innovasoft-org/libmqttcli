#ifndef __COMMON_H__
#define __COMMON_H__

#include "../../api/mqtt_common.h"

/** Program name */
#define PROGRAM_NAME      "hadev"
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
/** Short option: user id */
#define S_OPT_USERID        'I'
/** Long option: user id */
#define L_OPT_USERID        "userid"
/** Short option: user name */
#define S_OPT_USERNAME      'N'
/** Long option: user name */
#define L_OPT_USERNAME        "username"
/** Short option: password */
#define S_OPT_PASSWORD      'P'
/** Long option: password */
#define L_OPT_PASSWORD      "password"
/** Short option: verbose */
#define S_OPT_VERBOSE      'v'
/** Long option: verbose */
#define L_OPT_VERBOSE      "verbose"

#define LOG_EMERG 0
#define LOG_ALERT 1
#define LOG_CRIT 2
#define LOG_ERR 3
#define LOG_WARNING 4
#define LOG_NOTICE 5
#define LOG_INFO 6
#define LOG_DEBUG 7

//typedef enum device_state {
//  S_STOPPED = 0,
//  S_INITIALIZED = 1,
//  S_CONNECTED
//} device_state_t;

/** @brief Program context definition */
typedef struct program_ctx {
  /** IP to bind to */
  char ip[128];
  /** Port number to use */
  int port;
  /* Option reuse addr */
  int optval_reuse_addr;
  /* Non blocking */
  int non_blocking;
  /** Buffer size */
  int buffer_size;
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
} context_t;

#define IS_MULTICAST(IPADDR) ( (IPADDR & 0x000000E0) == 0x000000E0 )

void log_write(int level, char* filename, int line, char *fmt,...);

#define TOLOG(level, ...) log_write(level, __FILE__, __LINE__, __VA_ARGS__ )

#endif /* __COMMON_H__ */