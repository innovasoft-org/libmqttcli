#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"

#include "user_log.h"
#include "user_util.h"

static const char* const level_name[] = {"EMERG", "ALERT","CRIT", "ERR", "WARNING", "NOTICE", "INFO", "DEBUG"};
static uint8 max_level = LOG_EMERG;
static uint8_t log_buffer[256];

void log_init(uint8_t _max_level) {
  /* Turn off login into UART1 */
  system_set_os_print(0);

  if(_max_level < LOG_EMERG)
    return;

  max_level = _max_level;
}

void log_write(int level, char* filename, int line, char *message) {
  static int initialized = 0x00;
  int len = os_strlen(message);

  if(level > max_level)
    return;

  if( !initialized ) {
    initialized = 0x01;
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
  }

  /* Format the message */
  if(len > (ARRAYLEN(log_buffer) / 2) ) {
    /* message too big - ignored */
    len = os_sprintf(log_buffer, "%8s - %s:%d - ...\r\n", level_name[level], filename, line);
  }
  else {
    len = os_sprintf(log_buffer, "%8s - %s:%d - %s\r\n", level_name[level], filename, line, message);
  }

  /* Print the message */
  uart0_tx_buffer(log_buffer, len);
}
