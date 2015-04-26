/**
 ******************************************************************************
 * @addtogroup OpenPilot library
 * @brief      These files contain the code for stopwatch handling.
 *
 * @file       stopwatch.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      Generic pios_delay based stopwatch functions.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _STOPWATCH_H
#define _STOPWATCH_H
#include <stdint.h>
#include <pios_delay.h>
/////////////////////////////////////////////////////////////////////////////
// Global definitions
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Global Types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
    uint32_t raw;
    uint32_t resolution;
} stopwatch_t;

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////


static inline int32_t STOPWATCH_Init(uint32_t resolution, stopwatch_t *stopwatch)
{
    stopwatch->raw = PIOS_DELAY_GetRaw();
    stopwatch->resolution = resolution;
    return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// ! Resets the stopwatch
// ! \return < 0 on errors
/////////////////////////////////////////////////////////////////////////////
static inline int32_t STOPWATCH_Reset(stopwatch_t *stopwatch)
{
    stopwatch->raw = PIOS_DELAY_GetRaw();
    return 0; // no error
}

/////////////////////////////////////////////////////////////////////////////
// ! Returns current value of stopwatch
// ! \return stopwatch value
/////////////////////////////////////////////////////////////////////////////
static inline uint32_t STOPWATCH_ValueGet(stopwatch_t *stopwatch)
{
    uint32_t value = PIOS_DELAY_GetuSSince(stopwatch->raw);

    if (stopwatch > 1) {
        value = value / stopwatch->resolution;
    }
    return value;
}


/////////////////////////////////////////////////////////////////////////////
// Export global variables
/////////////////////////////////////////////////////////////////////////////


#endif /* _STOPWATCH_H */
