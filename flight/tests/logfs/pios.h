#ifndef PIOS_H
#define PIOS_H

/* PIOS board specific feature selection */
#include "pios_config.h"

#ifdef PIOS_INCLUDE_FLASH
#include <pios_flash.h>
#include <pios_flashfs.h>
#endif

#endif /* PIOS_H */
