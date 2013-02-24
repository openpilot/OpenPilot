#ifndef PIOS_BOARD_H_
#define PIOS_BOARD_H_

#ifdef USE_STM32103CB_AHRS
#include "STM32103CB_AHRS.h"
#elif USE_STM3210E_OP
#include "STM3210E_OP.h"
#elif USE_STM32103CB_PIPXTREME
#include "STM32103CB_PIPXTREME_Rev1.h"
#elif USE_STM32103CB_CC_Rev1
#include "STM32103CB_CC_Rev1.h"
#elif USE_STM32F2xx_INS
#include "STM32F2xx_INS.h"
#elif USE_STM32F4xx_OP
#include "STM32F4xx_Revolution.h"
#elif USE_STM32F4xx_OSD
#include "STM32F4xx_OSD.h"
#elif USE_STM32F4xx_RM
#include "STM32F4xx_RevoMini.h"
#elif USE_SIM_POSIX
#include "sim_posix.h"
#else
#error Board definition has not been provided.
#endif

#endif /* PIOS_BOARD_H_ */
