#ifndef ESPMISSINGINCLUDES_H
#define ESPMISSINGINCLUDES_H

#include <stdint.h>
#include <c_types.h>
#include <stdarg.h>

#ifndef malloc
#define malloc os_malloc
#endif

#ifndef zalloc
#define zalloc os_zalloc
#endif

#ifndef free
#define free os_free
#endif

#ifndef realloc
#define realloc os_realloc
#endif

#ifndef vsnprintf
#define vsnprintf ets_vsnprintf
#endif

#endif
