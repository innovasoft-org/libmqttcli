#include "user_util.h"
#include "user_config.h"

uint16_t htons (uint16_t x) {
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return UINT16_SWIPE (x);
#else
# error "What kind of system is this?"
#endif
}
