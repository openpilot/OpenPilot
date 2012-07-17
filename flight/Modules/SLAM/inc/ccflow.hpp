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

#define CC_PARTICLES 15
#define CC_PDEPTH 3
#define CC_ZOOMFACTOR 2
//#define CC_INITIAL Vec3f(5.0,5.0,5.0)
#define CC_INITIAL Vec3f(1.0,1.0,1.0)

#define CC_ADJFACTOR 0.985f
#define CC_MAXCOUNT 30
#define CC_ENDCOUNT 75

#define CC_BORDERWIDTH 1

#define CC_QUALITYMARGIN 100

typedef cv::Vec3f TransRot;

class CCFlow {


public:
	
	// methods
	CCFlow(cv::RNG *rnginit, cv::Mat* last[], cv::Mat* current[], int pyramidDepth,
		TransRot estTransrotation=cv::Vec3f(0,0,1000), CCFlow* oldflow=NULL, cv::Vec4s borders=cv::Vec4s(CC_BORDERWIDTH,CC_BORDERWIDTH,CC_BORDERWIDTH,CC_BORDERWIDTH),int depth=0);
	
	~CCFlow();

	TransRot transrotation();
	TransRot transrotation(cv:: Point3f position);
	TransRot transrotation(cv:: Point2f position);
	CCFlow* responsibleFlow(cv:: Point3f position);
	TransRot transrotationSmoothed(cv:: Point3f position);

	// finds the best transrotation between two templates of equal size via particle filter approach
	void particleMatch(cv::Mat test, cv::Mat reference, int particleNum, int generations, int stepFactor, TransRot initial, TransRot initialRange);
	
	// finds the best transrotation between two templates of equal size via tempered random jump gradient search
	void temperMatch(cv::Mat test, cv::Mat reference, int endCount, int maxCount, float adjFactor, TransRot initial, TransRot initialRange);

	void fullMatch(cv::Mat test, cv::Mat reference, float resolution, TransRot initial, TransRot initialRange);
	
	void gradientMatch(cv::Mat test, cv::Mat reference, int maxcount, TransRot initial, TransRot initialRange);

	// finds the mean square error between two templates of equal size of which one is transrotated
	float correlate(cv::Mat reference, cv::Mat test, cv::Mat rotMatrix);

	cv::Vec4f gradient(cv::Mat test, cv::Mat reference, TransRot position);

	// properties
	CCFlow *children[4]; // subcomponents
	
	cv::Vec2f translation;
	float rotation;
	float best;
	float worst;
	int iterations;
private:
	cv::Vec4i border;
	cv::RNG *rng;
	cv::Mat debug;
	int mydepth,maxdepth,width,height;

	cv::Point2f center;
	cv::Point2f corners[4];
	cv::Point2f subcenters[4];

};

#endif // CCFLOW_HPP

/**
 * @}
 * @}
 */
