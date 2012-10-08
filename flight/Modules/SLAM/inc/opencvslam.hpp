/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       opencvslam.hpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the task monitoring library
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

#ifndef OPENCVSLAM_HPP
#define OPENCVSLAM_HPP

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "rtslam.hpp"		// rtslam class
extern "C" {
	#include "openpilot.h"		// openpilot framework
	#include "slamsettings.h"	// module settings
	#include "attitudeactual.h"	// orientation in space
	#include "velocityactual.h"	// 3d velocity
	#include "positionactual.h"	// 3d position
	#include "opencvslam.h"		// c wrapper
	#include "backgroundio.h"	// c wrapper
	#include "rtslam.h"		// c wrapper
};

class OpenCVslam {


public:
	OpenCVslam(SLAMSettingsData * newsettings);
	void run();

protected:
	SLAMSettingsData *settings;
	AttitudeActualData attitudeActual;
	VelocityActualData velocityActual;
	PositionActualData positionActual;
	IplImage *currentFrame, *lastFrame;
	uint32_t frame;
	CvCapture *VideoSource;
	CvVideoWriter *VideoDest;
	RTSlam *rtslam;
};

#endif // OPENCVSLAM_HPP

/**
 * @}
 * @}
 */
