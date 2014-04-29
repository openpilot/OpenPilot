/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Reuseable math functions
 * @{
 *
 * @file       mathmisc.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Reuseable math functions
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

#ifndef MATHMISC_H
#define MATHMISC_H

// returns min(boundary1,boundary2) if val<min(boundary1,boundary2)
// returns max(boundary1,boundary2) if val>max(boundary1,boundary2)
// returns val if min(boundary1,boundary2)<=val<=max(boundary1,boundary2)
static inline float boundf(float val, float boundary1, float boundary2)
{
    if (boundary1 > boundary2) {
        if (!(val >= boundary2)) {
            return boundary2;
        } else if (!(val <= boundary1)) {
            return boundary1;
        }
    } else {
        if (!(val >= boundary1)) {
            return boundary1;
        } else if (!(val <= boundary2)) {
            return boundary2;
        }
    }
    return val;
}

#endif /* MATHMISC_H */
