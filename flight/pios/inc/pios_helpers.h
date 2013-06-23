/**
 ******************************************************************************
 *
 * @file       pios_helpers.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Header for helper functions/macro definitions
 *
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

#ifndef PIOS_HELPERS_H
#define PIOS_HELPERS_H


/**
 * @brief return the number of elements contained in the array x.
 * @param[in] x the array
 * @return  number of elements in x.
 *
 */
#define NELEMENTS(x)       (sizeof(x) / sizeof((x)[0]))


/**
 * @brief Compiler barrier: Disables compiler load/store reordering across the barrier
 *
 */
#define COMPILER_BARRIER() asm volatile ("" ::: "memory")

// Memory barriers:
// Note that on single core Cortex M3 & M4, the is generally no need to use a processor memory barrier instruction such as DMB.
// See http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf
// However, it makes sense to use these if we want to reduce issues if we ever port to a multicore processor in the future.
// An important exception for STM32 is when setting up the DMA engine - see the above reference for details.

/**
 * @brief  Read Acquire memory barrier
 */
#define READ_MEMORY_BARRIER()  COMPILER_BARRIER()
/**
 * @brief Write Release memory barrier
 */
#define WRITE_MEMORY_BARRIER() COMPILER_BARRIER()

/**
 * @brief Full fence memory barrier
 */
#define MEMORY_BARRIER()       COMPILER_BARRIER()

// For future multicore ARM v7 or later:
// The above three macros would be replaced with: asm volatile("dmb":::"memory")
#define BitCheck(mask,bitnum) (mask && 1 << bitnum)

#endif // PIOS_HELPERS_H
