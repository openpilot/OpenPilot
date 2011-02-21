/**
 ******************************************************************************
 *
 * @file       homelocationutil.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Utilities to find the location of openpilot GCS files:
 *             - Plugins Share directory path
 *
 * @brief      Home location utility functions
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

#include "homelocationutil.h"

#include <stdint.h>
#include <QDebug>
#include <QDateTime>

#include "coordinateconversions.h"
#include "worldmagmodel.h"

namespace Utils {

	HomeLocationUtil::HomeLocationUtil()
    {
//        Initialize();
    }

	int HomeLocationUtil::getDetails(double LLA[3], double ECEF[3], double RNE[9], double Be[3])
    {
		QDateTime dt = QDateTime::currentDateTime().toUTC();

//		double current_altitude = obj->getField("Altitude")->getDouble();

		CoordinateConversions().LLA2ECEF(LLA, ECEF);
		CoordinateConversions().RneFromLLA(LLA, (double (*)[3])RNE);
		if (WorldMagModel().GetMagVector(LLA, dt.date().month(), dt.date().day(), dt.date().year(), Be) < 0)
			return -1;

		return 0;   // OK
    }

}
