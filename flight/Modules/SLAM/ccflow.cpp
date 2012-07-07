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
#include "opencv/highgui.h"

using namespace cv;





CCFlow::CCFlow(cv::RNG *rnginit, cv::Mat* last[], cv::Mat* current[], int pyramidDepth, TransRot estTransrotation, CCFlow* oldflow, cv::Vec4s borders, int depth) {

	rng = rnginit;
	border=borders;

	if (depth == 0) {
		TransRot tr;
		// initialize transrotation
		if (estTransrotation[2] < 999) {
			tr = estTransrotation;
		} else if (oldflow) {
			tr = oldflow->transrotation();
		} else {
			tr = Vec3f(0,0,0);
		}
		tr = Vec3f(0,0,0);
		particleMatch(*last[0],*current[0],CC_PARTICLES,CC_PDEPTH,CC_ZOOMFACTOR,tr,CC_INITIAL);
	}

}

// return global flow of this flow segment
TransRot CCFlow::transrotation() {
	return Vec3f(translation[0],translation[1],rotation);
}



void CCFlow::particleMatch(cv::Mat test, cv::Mat reference,
				int particleNum, int generations, int stepFactor,
				TransRot initial, TransRot initialRange )
{

	// create particles
	TransRot particles[particleNum];
	int bestMatch = -1;
	float bestMatchScore = 1000000000; // huge
	float worstMatchScore = 0; // small

	for (int g=0;g<generations;g++) {
		for (int t=0;t<particleNum;t++) {
			particles[t] = Vec3f( initial[0], initial[1],
					rng->gaussian(initialRange[2])+initial[2]);
			Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),(particles[t])[2],1.0f);
			rotMatrix.at<double>(0,2)+=(particles[t])[0];
			rotMatrix.at<double>(1,2)+=(particles[t])[1];
			fprintf(stderr,",,,x: %f y: %f r: %f\n",(particles[t])[0],(particles[t])[1],(particles[t])[2]);

			float squareError = correlate(reference,test,rotMatrix);
			if (squareError<bestMatchScore) {
				bestMatch = t;
				bestMatchScore = squareError;
			}
			if (squareError>worstMatchScore) worstMatchScore=squareError;
		}
		if (bestMatch!=-1) {
			initial[0] = (particles[bestMatch])[0];
			initial[1] = (particles[bestMatch])[1];
			initial[2] = (particles[bestMatch])[2];
			bestMatch = -1;
			fprintf(stderr,"YYYx: %f y: %f r: %f\n",initial[0],initial[1],initial[2]);
		}
		for (int t=0;t<particleNum;t++) {
			particles[t] = Vec3f( rng->gaussian(initialRange[0])+initial[0],
					rng->gaussian(initialRange[1])+initial[1],
					initial[2]);
			Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),(particles[t])[2],1.0f);
			rotMatrix.at<double>(0,2)+=(particles[t])[0];
			rotMatrix.at<double>(1,2)+=(particles[t])[1];
			fprintf(stderr,"...x: %f y: %f r: %f\n",(particles[t])[0],(particles[t])[1],(particles[t])[2]);

			float squareError = correlate(reference,test,rotMatrix);
			//fprintf(stderr,"%f\n",squareError);
			if (squareError<bestMatchScore) {
				bestMatch = t;
				bestMatchScore = squareError;
			}
			if (squareError>worstMatchScore) worstMatchScore=squareError;
		}
		if (bestMatch!=-1) {
			initial[0] = (particles[bestMatch])[0];
			initial[1] = (particles[bestMatch])[1];
			initial[2] = (particles[bestMatch])[2];
			bestMatch = -1;
			fprintf(stderr,"XXXx: %f y: %f r: %f\n",initial[0],initial[1],initial[2]);
		}
		initialRange[0] /=  stepFactor;
		initialRange[1] /=  stepFactor;
		initialRange[2] /=  stepFactor;
	}
	translation = Vec2f(initial[0],initial[1]);
	rotation = initial[2];
	best = bestMatchScore;
	worst = worstMatchScore;
}


float CCFlow::correlate(cv::Mat reference, cv::Mat test, cv::Mat rotMatrix) {

	double * m = (double*)rotMatrix.data;
	float squareError=0;
	int   pixels=0;

Mat debug=test.clone()*0;
	for (int y=0;y<reference.rows;y++) {
		for (int x=0;x<reference.cols;x++) {
			int tx = (float)(16.0f*(m[0]*x + m[1]*y + m[2]));
			int ty = (float)(16.0f*(m[3]*x + m[4]*y + m[5]));
			// check whether source pixel is within outer boundaries
			if ((tx/16)>=0-border[0] && (tx/16)+1<test.cols+border[1] && (ty/16)>=0-border[2] && (ty/16)+1<test.rows+border[3]) {
				pixels++;
				Vec3i tmp = reference.at<Vec3b>(y,x);
				Vec3i a1 = test.at<Vec3b>(ty/16,tx/16);
				Vec3i a2 = test.at<Vec3b>(ty/16,tx/16 +1);
				Vec3i b1 = test.at<Vec3b>(ty/16 +1,tx/16);
				Vec3i b2 = test.at<Vec3b>(ty/16 +1,tx/16 +1);
				Vec3i a = (16-(tx%16))*a1 + (tx%16)*a2;
				Vec3i b = (16-(tx%16))*b1 + (tx%16)*b2;
				Vec3i c = ((16-(ty%16))*a + (ty%16)*b);
				c[0]>>=8;
				c[1]>>=8;
				c[2]>>=8;
				tmp -= c;
				//debug.at<Vec3b>(y,x) = Vec3b(tmp[0]*tmp[0],tmp[1]*tmp[1],tmp[2]*tmp[2]);
				debug.at<Vec3b>(y,x) = c;

				squareError += tmp[0]*tmp[0] + tmp[1]+tmp[1] + tmp[2]*tmp[2];
			}
		}
	}
	if (pixels==0) return reference.rows*reference.cols*3*256;
//#if 0
	pyrUp(debug,debug);
	pyrUp(debug,debug);
	pyrUp(debug,debug);
	Mat x1;
	pyrUp(reference,x1);
	pyrUp(x1,x1);
	pyrUp(x1,x1);
	imshow("debug",x1);
	waitKey(0);
	pyrUp(test,x1);
	pyrUp(x1,x1);
	pyrUp(x1,x1);
	imshow("debug",x1);
	waitKey(0);
	imshow("debug",debug);
	waitKey(0);
//#endif

	return squareError/pixels;
}




/* End of file. */
