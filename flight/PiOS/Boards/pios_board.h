/*
 * Generic pios_board.h header.
 * There is no #ifdef guard here since only one of headers should be included.
 * All they share the same guard define to prevent double inclusion.
 */

#if USE_STM32103CB_CC_Rev1
#include "STM32103CB_CC_Rev1.h"
#elif USE_STM32103CB_OPLINKMINI
#include "STM32103CB_OPLinkMini_Rev1.h"
#elif USE_STM32F4xx_RM
#include "STM32F4xx_Revolution.h"
#elif USE_STM32F4xx_OSD
#include "STM32F4xx_OSD.h"
#elif USE_STM32F4xx_OP
#include "STM32F4xx_SensorTest.h"
#elif USE_SIM_POSIX
#include "sim_posix.h"
#else
#error Board definition has not been provided.
#endif
