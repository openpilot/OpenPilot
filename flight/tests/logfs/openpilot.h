#include <stdbool.h>

#define PIOS_Assert(x) if (!(x)) { while (1) ; }

#define PIOS_DEBUG_Assert(x) PIOS_Assert(x)
