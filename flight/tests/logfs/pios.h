#ifndef PIOS_H
#define PIOS_H

/* PIOS Feature Selection */
#include "pios_config.h"

#ifdef PIOS_INCLUDE_FREERTOS
/* FreeRTOS Includes */
#include "FreeRTOS.h"
#endif

#ifdef PIOS_INCLUDE_FLASH
#include <pios_flash.h>
#include <pios_flashfs.h>
#endif

#endif /* PIOS_H */
