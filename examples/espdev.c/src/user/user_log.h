#ifndef __USER_LOG_H__
#define __USER_LOG_H__

#include <c_types.h>
#include "user_config.h"

/** @note Log levels based on GNU. */
#define LOG_EMERG   (uint8) (0)
#define LOG_ALERT   (uint8) (1)
#define LOG_CRIT    (uint8) (2)
#define LOG_ERR     (uint8) (3)
#define LOG_WARNING (uint8) (4)
#define LOG_NOTICE  (uint8) (5)
#define LOG_INFO    (uint8) (6)
#define LOG_DEBUG   (uint8) (7)

#define TOLOG( LEVEL, MSG ) log_write( LEVEL, MSG );

void log_init(uint8_t _max_level);
void log_write(uint8_t level, const char* msg);

#endif //__USER_LOG_H___
