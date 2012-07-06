/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       ccflow.cpp
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

#include "ccflow.hpp"
#include "pyramidsenhanced.hpp"

using namespace cv;





CCFlow::CCFlow(cv::InputArray last[], cv::InputArray current[], int pyramidDepth, TransRot estTransrotation=Vec3f(0,0,256), CCFlow* oldflow=NULL, cv::Vec4s borders=Vec4s(0,0,0,0)) {

	border=borders;

	if (pyramidDepth == 1) {
		// initialize transrotation
		if (estTransrotation[2] != 256) {
			transrotation = estTransotation;
		} else if (oldflow) {
			transrotation = oldflow->transrotation;
		} else {
			transrotation = Vec3f(0,0,0);
		}
		particleMatch(last[0],current[0]);
		
	}

}

// return global flow of this flow segment
TransRot CCFlow::transrotation() {
	return Vec3f(translation[0],translation[1],rotation);
}



TransRot particleRandom(TransRot initial, TransRot initialRange) {
	return Vec3f( RNG:gaussian(initialRange[0])+initial[0],
		RNG:gaussian(initialRange[1])+initial[1],
		RNG:gaussian(initialRange[2])+initial[2]);
}


void CCFlow::particleMatch(cv::Mat& test, cv::Mat& reference,
				int particleNum, int generations,
				TransRot initial, TransRot initialRange )
{

	// create particles
	TransRot particles[particleNum];
	int bestMatch = -1;
	float bestMatchScore = 1000000000; // huge

	for (int t=0;t<particleNum;t++) {
		particle[t] = particleRandom(initial,initialRange);
		Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),particle[t][2]);
		rotMatrix.at(0,2)+=particle[t][0];
		rotMatrix.at(1,2)+=particle[t][1];

		float squareError = correlate(reference,test,rotMatrix);
		if (squareError<bestMatchScore) {
			bestMatch = t;
			bestMatchScore = squareError;
		}
	}
}


float CCFlow::correlate(cv::Mat& reference, cv::Mat& test, cv::Mat& rotMatrix) {

	float M11=rotMatrix.at(0,0);
	float M12=rotMatrix.at(0,1);
	float M13=rotMatrix.at(0,2);
	float M21=rotMatrix.at(1,0);
	float M22=rotMatrix.at(1,1);
	float M23=rotMatrix.at(1,2);
	float squareError=0;
	int   pixels=0;
	for (int y=0;y<reference.rows;y++) {
		for (x=0;x<reference.cols;x++) {
			int tx = (float)(M11*x + M12*y + M13);
			int ty = (float)(M21*x + M22*y + M23);
			// check whether source pixel is within outer boundaries
			if (tx>=0-border[0] && tx<reference.cols+border[1] && ty>=0-border[2] && ty<reference.rows+border[3]) {
				pixels++;
				for (cv=0;cv<3;cv++) {
					float tmp = reference.at(x,y,cv)-test.at(tx,ty,cv);
					squareError += tmp*tmp;
				}
		}
	}

	return squareErrors/pixels;
}




/* End of file. */
