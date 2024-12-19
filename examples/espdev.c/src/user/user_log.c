#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "driver/uart.h"

#include "user_log.h"

static uint8 max_level = LOG_EMERG;

void log_init(uint8_t _max_level) {
  /* Turn off login into UART1 */
  system_set_os_print(0);

  if(_max_level < LOG_EMERG)
    return;

  max_level = _max_level;
}

void log_write(uint8_t level, const char* msg ) {
  static int initialized = 0x00;
  int len = strlen(msg);

  if(level > max_level)
    return;
 
  if( !initialized ) {
    initialized = 0x01;
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
  }
  uart0_tx_buffer((uint8*)msg,len);
}
