#ifndef __USER_UTIL_H__
#define __USER_UTIL_H__

#include <c_types.h>

/* Calculating length of defined array */
#define ARRAYLEN(ARRAY) ( sizeof(ARRAY) / sizeof(ARRAY[0]) )

/* checking if specified argument is a digit */
#define ISDIGIT(V) ( V==0x30 || V==0x31 || V==0x32 || V==0x33 || V==0x34 || V==0x35 || V==0x36 || V==0x37 || V==0x38 || V==0x39 )

/* Swap bytes in 16-bit value */
#define UINT16_SWIPE(x)					\
  ((uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

/* Swap bytes in 32-bit value.  */
#define UINT32_SWIPE(x)					\
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)	\
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))

uint16_t htons (uint16_t x);

#endif /* #define __USER_UTIL_H__*/


