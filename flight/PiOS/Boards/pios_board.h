#ifndef PIOS_BOARD_H
#define PIOS_BOARD_H

#if USE_STM32103CB_CC_Rev1
#include "STM32103CB_CC_Rev1.h"
#elif USE_STM32103CB_OPLINKMINI
#include "STM32103CB_OPLinkMini_Rev1.h"
#elif USE_STM32F4xx_RM
#include "STM32F4xx_RevoMini.h"
#elif USE_STM32F4xx_OSD
#include "STM32F4xx_OSD.h"
#elif USE_STM32F4xx_OP
#include "STM32F4xx_Revolution.h"
#elif USE_SIM_POSIX
#include "sim_posix.h"
#else
#error Board definition has not been provided.
#endif

#endif /* PIOS_BOARD_H */
