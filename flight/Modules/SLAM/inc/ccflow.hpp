/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       ccflow.hpp
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

#ifndef CCFLOW_HPP
#define CCFLOW_HPP

#include "opencv/cv.h"


// transrotation is a vector specifying
// X: displacement in horizontal direction (pixels)
// Y: displacement in vertical (pixels)
// R: rotation in degrees (OpenCV operates in degrees)

#define CC_PARTICLES 10
#define CC_PDEPTH 3

typedef cc::Vec3f TransRot

class CCFlow {


public:
	
	// methods
	CCFlow(cv::InputArray last[], cv::InputArray current[], int pyramidDepth,
		TransRot estTransrotation=Vec3f(0,0,0), CCFlow* oldflow=NULL, cv::Vec4s borders=Vec4s(0,0,0,0));

	TransRot transrotation();
	TransRot transrotation(cv:: Point2i position);
	TransRot transrotation(cv:: Point3i position);

	// finds the best transrotation between two templates of equal size via particle filter approach
	void particleMatch(cv::Mat& test, cv::Mat& reference, int particleNum, int generations, TransRot initial, TransRot initialRange);

	// finds the mean square error between two templates of equal size of which one is transrotated
	float correlate(cv::Mat& reference, cv::Mat& test, cv::Mat& rotMatrix);

	// properties
	CCFlow[][] quarters; // subcomponents
	
	cv::Size origSize; // size of area before shrinking
	cv::Mat area; // mat after rotation (and possibly shrinkage)
private:
	cv::Vec2f translation;
	float rotation;
	cv::Vec4i border;


};

#endif // CCFLOW_HPP

/**
 * @}
 * @}
 */
