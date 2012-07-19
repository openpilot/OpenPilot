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
	border = borders;
	mydepth = depth;
	maxdepth = pyramidDepth;

	TransRot tr;
	// initialize transrotation
	if (estTransrotation[2] < 999) {
		tr = estTransrotation;
	} else if (oldflow) {
		tr = oldflow->transrotation();
	} else {
		tr = Vec3f(0,0,0);
	}
	//tr = Vec3f(0,0,0);
	Mat ref=(*current[mydepth])(Rect(border[0], border[1], current[mydepth]->cols-(border[0]+border[2]), current[mydepth]->rows-(border[1]+border[3])));
	Mat tst=(*last[mydepth])(Rect(border[0], border[1], last[mydepth]->cols-(border[0]+border[2]), last[mydepth]->rows-(border[1]+border[3])));
	width  = ref.cols;
	height = ref.rows;

/*fprintf(stderr,"Doing subcheck on level %i subsize %i,%i of %i,%i from %i,%i to %i,%i\n",
	depth,
	ref.cols,ref.rows,
	current[mydepth]->cols,current[mydepth]->rows,
	border[0],border[1],
	current[mydepth]->cols-border[2],current[mydepth]->rows-border[3]);*/



	//particleMatch(tst,ref,CC_PARTICLES,CC_PDEPTH,CC_ZOOMFACTOR,tr,CC_INITIAL);
	//temperMatch(tst,ref,CC_ENDCOUNT,CC_MAXCOUNT,CC_ADJFACTOR,tr,CC_INITIAL);
	//fullMatch(tst,ref,1.0/16.0,Vec3f(0,0,0),Vec3f(10,10,10));
	//gradientMatch(tst,ref,Vec3f(0,0,0),Vec3f(10,10,10));
	iterations=0;
	if (depth==0) {
		gradientMatch(tst,ref,50,tr,Vec3f(1,1,1));
	} else {
		gradientMatch(tst,ref,20,tr,Vec3f(0.5,0.5,0.1));
	}
	//if (worst-best < CC_QUALITYMARGIN && estTransrotation[2]<999 ) {
	//	translation=Vec2f(estTransrotation[0],estTransrotation[1]);
	//	rotation=estTransrotation[2];
	//}

	center = Point2f(
		border[0] + 0.5*(last[mydepth]->cols-(border[0]+border[2])),
		border[1] + 0.5*(last[mydepth]->rows-(border[1]+border[3]))
		);
	if ((mydepth+1)<maxdepth) {
		Vec4i newborder = border * 2;
		Point2f newcenter = Point2f(
			newborder[0] + 0.5*(last[mydepth+1]->cols-(newborder[0]+newborder[2])),
			newborder[1] + 0.5*(last[mydepth+1]->rows-(newborder[1]+newborder[3]))
			);
		corners[0] = Point2f(newborder[0], newborder[1]);
		corners[1] = Point2f(last[mydepth+1]->cols-newborder[2], newborder[1]);
		corners[2] = Point2f(newborder[0], last[mydepth+1]->rows-newborder[3]);
		corners[3] = Point2f(last[mydepth+1]->cols-newborder[2], last[mydepth+1]->rows-newborder[3]);

		subcenters[0] = corners[0] + 0.5 * (newcenter-corners[0]);
		subcenters[1] = corners[1] + 0.5 * (newcenter-corners[1]);
		subcenters[2] = corners[2] + 0.5 * (newcenter-corners[2]);
		subcenters[3] = corners[3] + 0.5 * (newcenter-corners[3]);

		/*fprintf(stderr,"subcenters are at %f/%f, %f/%f, %f/%f, %f/%f \n",
			subcenters[0].x,subcenters[0].y,
			subcenters[1].x,subcenters[1].y,
			subcenters[2].x,subcenters[2].y,
			subcenters[3].x,subcenters[3].y);*/

		TransRot shift[4] = {
			transrotation(Point3f(subcenters[0].x/2,subcenters[0].y/2,mydepth)),
			transrotation(Point3f(subcenters[1].x/2,subcenters[1].y/2,mydepth)),
			transrotation(Point3f(subcenters[2].x/2,subcenters[2].y/2,mydepth)),
			transrotation(Point3f(subcenters[3].x/2,subcenters[3].y/2,mydepth))
		};

		children[0] = new CCFlow( rnginit, last, current, maxdepth, shift[0], oldflow?oldflow->children[0]:NULL,
					Vec4s( corners[0].x, corners[0].y, last[mydepth+1]->cols - newcenter.x, last[mydepth+1]->rows - newcenter.y ), mydepth+1);
		children[1] = new CCFlow( rnginit, last, current, maxdepth, shift[1], oldflow?oldflow->children[1]:NULL,
					Vec4s( newcenter.x,     corners[1].y, last[mydepth+1]->cols - corners[1].x, last[mydepth+1]->rows - newcenter.y ), mydepth+1);
		children[2] = new CCFlow( rnginit, last, current, maxdepth, shift[2], oldflow?oldflow->children[2]:NULL,
					Vec4s( corners[2].x, newcenter.y, last[mydepth+1]->cols - newcenter.x, last[mydepth+1]->rows - corners[2].y ), mydepth+1);
		children[3] = new CCFlow( rnginit, last, current, maxdepth, shift[3], oldflow?oldflow->children[3]:NULL,
					Vec4s( newcenter.x,     newcenter.y, last[mydepth+1]->cols - corners[3].x, last[mydepth+1]->rows - corners[3].y ), mydepth+1);


	} else {
		children[0]=NULL;
		children[1]=NULL;
		children[2]=NULL;
		children[3]=NULL;
	}

}

// destructor
CCFlow::~CCFlow() {
	if (children[0]) delete children[0];
	if (children[1]) delete children[1];
	if (children[2]) delete children[2];
	if (children[3]) delete children[3];
}


// return global flow of the current flow segment
TransRot CCFlow::transrotation() {
	return Vec3f(translation[0],translation[1],rotation);
}

// return local flow of the smallest flow segment
TransRot CCFlow::transrotation(cv:: Point2f position) {
	return transrotation(Point3f(position.x,position.y,maxdepth-1));
}


CCFlow* CCFlow::responsibleFlow(cv:: Point3f position) {
	Point2f pos(position.x,position.y);
	int depth = position.z;
	// calculate which subdivision is responsible
	if (depth>mydepth && mydepth+1<maxdepth) {
		for (int t=mydepth; t<depth; t++) {
			pos = pos * 0.5;
		}
		if (pos.y<center.y) {
			if (pos.x<center.x) {
				return children[0]->responsibleFlow(position);
			} else {
				return children[1]->responsibleFlow(position);
			}
		} else {
			if (pos.x<center.x) {
				return children[2]->responsibleFlow(position);
			} else {
				return children[3]->responsibleFlow(position);
			}
		}
	} else {
		return this;
	}
}

TransRot CCFlow::transrotationSmoothed(cv:: Point3f position) {
	CCFlow* ref = responsibleFlow(position);
	Point2f pos(position.x,position.y);
	int depth = position.z;
	int disp=1;
	// correct scale to find out tile size
	for (int t=ref->mydepth; t<depth; t++) {
		pos = pos * 0.5;
		disp *=2;
	}
	Point2f xpos(pos.x-(ref->width/2),pos.y-(ref->height/2));
	CCFlow* A1 = responsibleFlow(Point3f(xpos.x,xpos.y,ref->mydepth));
	CCFlow* A2 = responsibleFlow(Point3f(xpos.x+ref->width,xpos.y,ref->mydepth));
	CCFlow* B1 = responsibleFlow(Point3f(xpos.x,xpos.y+ref->height,ref->mydepth));
	CCFlow* B2 = responsibleFlow(Point3f(xpos.x+ref->width,xpos.y+ref->height,ref->mydepth));
	float tx = (xpos.x-A1->border[0])/ref->width;
	float ty = (xpos.y-A1->border[1])/ref->height;
	//fprintf(stderr, " tx,tz %f,%f\n",tx,ty);
	if (tx<0) tx=0;
	if (tx>1) tx=1;
	if (ty<0) ty=0;
	if (ty>1) ty=1;
	TransRot a = (1-tx)*TransRot(A1->translation[0],A1->translation[1],A1->rotation) + tx*TransRot(A2->translation[0],A2->translation[1],A2->rotation);
	TransRot b = (1-tx)*TransRot(B1->translation[0],B1->translation[1],B1->rotation) + tx*TransRot(B2->translation[0],B2->translation[1],B2->rotation);
	TransRot c = (1-ty)*a + ty*b;
	Point2f cn((1-tx)*A1->center.x + tx*B2->center.x,(1-ty)*A1->center.y + ty*B2->center.y);
	Mat rotMatrix = getRotationMatrix2D(cn,c[2],1.0f);
	rotMatrix.at<double>(0,2)+=c[0];
	rotMatrix.at<double>(1,2)+=c[1];
	double * m = (double*)rotMatrix.data;
	Point2f source(
		m[0]*pos.x + m[1]*pos.y + m[2],
		m[3]*pos.x + m[4]*pos.y + m[5]
		);

	Point2f trans(source-pos);
	return Vec3f(disp*trans.x,disp*trans.y,c[2]);

}

// return local flow of the given flow segment
TransRot CCFlow::transrotation(cv:: Point3f position) {
	// make sure we are esponsible
	CCFlow* r=responsibleFlow(position);
	if (this!=r) return r->transrotation(position);

	Point2f pos(position.x,position.y);
	int depth = position.z;

	// we are responsible
	// adjust position for correct scale (even if the flow hasnt bee calculatd to that scale)
	int disp=1;
	for (int t=mydepth; t<depth; t++) {
		pos = pos * 0.5;
		disp *=2;
	}
	Mat rotMatrix = getRotationMatrix2D(center,rotation,1.0f);
	rotMatrix.at<double>(0,2)+=translation[0];
	rotMatrix.at<double>(1,2)+=translation[1];
	double * m = (double*)rotMatrix.data;
	Point2f source(
		m[0]*pos.x + m[1]*pos.y + m[2],
		m[3]*pos.x + m[4]*pos.y + m[5]
		);

	Point2f trans(source-pos);
	return Vec3f(disp*trans.x,disp*trans.y,rotation);
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
		for (int type=0;type<5;type++) {
			for (int t=0;t<particleNum;t++) {
				particles[t] = Vec3f( initial[0], initial[1], initial[2] );
				if (type==0 || type==1) {
					(particles[t])[2] += rng->gaussian(initialRange[2]);
				}
				if (type==0 || type==2 || type==3) {
					(particles[t])[0] += rng->gaussian(initialRange[0]);
				}
				if (type==0 || type==2 || type==4) {
					(particles[t])[1] += rng->gaussian(initialRange[1]);
				}
				Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),(particles[t])[2],1.0f);
				rotMatrix.at<double>(0,2)+=(particles[t])[0];
				rotMatrix.at<double>(1,2)+=(particles[t])[1];
				//fprintf(stderr,",,,x: %f y: %f r: %f\n",(particles[t])[0],(particles[t])[1],(particles[t])[2]);

				float squareError = correlate(reference,test,rotMatrix);
				if (squareError>0 && squareError<bestMatchScore) {
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
				fprintf(stderr,"gen %i, type %i, score: %f\tx: %f y: %f r: %f\n",g,type,bestMatchScore,initial[0],initial[1],initial[2]);
			}
		}
		initialRange[0] /=  stepFactor;
		initialRange[1] /=  stepFactor;
		initialRange[2] /=  stepFactor;
	}
	Mat d;
	pyrUp (reference,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug1",d);
	pyrUp (test,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug2",d);
	Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),initial[2],1.0f);
	rotMatrix.at<double>(0,2)+=initial[0];
	rotMatrix.at<double>(1,2)+=initial[1];
	warpAffine(test,d,rotMatrix,test.size(),INTER_LINEAR+WARP_INVERSE_MAP);
	pyrUp (d,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug3",d);
	fprintf(stderr,"x: %f y: %f r: %f - %f > %f\n",initial[0],initial[1],initial[2],bestMatchScore,worstMatchScore);
	waitKey(1);
	translation = Vec2f(initial[0],initial[1]);
	rotation = initial[2];
	best = bestMatchScore;
	worst = worstMatchScore;
}

void CCFlow::temperMatch(cv::Mat test, cv::Mat reference, int endCount, int maxCount, float adjFactor, TransRot initial, TransRot initialRange) {

	float bestMatchScore = 1000000000; // huge
	float worstMatchScore = 0; // small
	int count=endCount;
	int dcount=maxCount;
	int improvements=0;
	TransRot current = initial;
	int type = 0;

	while (count-- > 0 && dcount-- > 0) {

		Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),current[2],1.0f);
		rotMatrix.at<double>(0,2)+=current[0];
		rotMatrix.at<double>(1,2)+=current[1];

		float squareError = correlate(reference,test,rotMatrix);
		if (squareError>worstMatchScore) worstMatchScore=squareError;
		if (squareError>0 && squareError<bestMatchScore) {
			bestMatchScore = squareError;
			initial = current;
			count = endCount;
			improvements++;
		}

		current = Vec3f( initial[0], initial[1], initial[2] );
		if (type==0 || type==1) {
			(current)[2] += rng->gaussian(initialRange[2]);
		}
		if (type==0 || type==2 || type==3) {
			(current)[0] += rng->gaussian(initialRange[0]);
		}
		if (type==0 || type==2 || type==4) {
			(current)[1] += rng->gaussian(initialRange[1]);
		}
		initialRange[0] *=  adjFactor;
		initialRange[1] *=  adjFactor;
		initialRange[2] *=  adjFactor;
		type++;
		if (type>4) type=0;

	}
	#if 0
	fprintf(stderr,"factor is now %f %f %f\n",initialRange[0],initialRange[1],initialRange[2]);
	Mat d;
	pyrUp (reference,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug1",d);
	pyrUp (test,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug2",d);
	Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),initial[2],1.0f);
	rotMatrix.at<double>(0,2)+=initial[0];
	rotMatrix.at<double>(1,2)+=initial[1];
	warpAffine(test,d,rotMatrix,test.size(),INTER_LINEAR+WARP_INVERSE_MAP);
	pyrUp (d,d);
	pyrUp (d,d);
	pyrUp (d,d);
	imshow("debug3",d);
	fprintf(stderr,"x: %f y: %f r: %f - %f > %f -- %i\n",initial[0],initial[1],initial[2],bestMatchScore,worstMatchScore,improvements);
	waitKey(1);
	#endif
	translation = Vec2f(initial[0],initial[1]);
	rotation = initial[2];
	best = bestMatchScore;
	worst = worstMatchScore;

}

void CCFlow::fullMatch(cv::Mat test, cv::Mat reference, float resolution, TransRot initial, TransRot initialRange) {

	float bestMatchScore = 1000000000; // huge
	float worstMatchScore = 0; // small


	Mat results(2*initialRange[0]/resolution,2*initialRange[1]/resolution,CV_32FC1);

	for (int x=0;x<2*initialRange[0]/resolution;x++) {
		for (int y=0;y<2*initialRange[1]/resolution;y++) {
			float tx=x*resolution-initialRange[0];
			float ty=y*resolution-initialRange[1];

			Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),0,1.0f);
			rotMatrix.at<double>(0,2)+=tx;
			rotMatrix.at<double>(1,2)+=ty;

			float squareError = correlate(reference,test,rotMatrix);
			if (squareError>worstMatchScore) worstMatchScore=squareError;
			if (squareError>0 && squareError<bestMatchScore) bestMatchScore = squareError;

			if (squareError>0) {
				results.at<float>(y,x)=squareError;
			} else {
				results.at<float>(y,x)=10000;
			}
		}

	}
	//Mat debug;
	pyrUp (reference,debug);
	pyrUp (debug,debug);
	pyrUp (debug,debug);
	imshow("debug1",debug);
	pyrUp (test,debug);
	pyrUp (debug,debug);
	pyrUp (debug,debug);
	imshow("debug2",debug);
	normalize(results,debug,0,200,NORM_MINMAX);
	for (int x=8;x<debug.cols;x+=16) {
		for (int y=8;y<debug.rows;y+=16) {
			Point gradient(results.at<float>(y,x+1)-results.at<float>(y,x-1),results.at<float>(y+1,x)-results.at<float>(y-1,x));
			line(debug,Point(x,y),Point(x,y)+gradient,CV_RGB(0,0,0));
		}
	}
	imshow("debug3",debug);
	waitKey(1);
	//#endif

}

Vec4f CCFlow::gradient(cv::Mat test, cv::Mat reference, TransRot position) {
	Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),position[2],1.0f);
	rotMatrix.at<double>(0,2)+=position[0];
	rotMatrix.at<double>(1,2)+=position[1];
	float r0 = correlate(reference,test,rotMatrix);
	if (r0<0) return Vec4f(0,0,0,-1);
	rotMatrix.at<double>(0,2)+=1./16.;
	float rx = correlate(reference,test,rotMatrix);
	if (rx<0) return Vec4f(0,0,0,-1);
	rotMatrix.at<double>(0,2)-=1./16.;
	rotMatrix.at<double>(1,2)+=1./16.;
	float ry = correlate(reference,test,rotMatrix);
	if (ry<0) return Vec4f(0,0,0,-1);
	rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),position[2]+1./16.,1.0f);
	rotMatrix.at<double>(0,2)+=position[0];
	rotMatrix.at<double>(1,2)+=position[1];
	float rr = correlate(reference,test,rotMatrix);
	//rr=r0;
	if (rr<0) return Vec4f(0,0,0,-1);

	float len=sqrt((rx-r0)*(rx-r0)+(ry-r0)*(ry-r0)+(rr-r0)*(rr-r0));
	return Vec4f((r0-rx)/len,(r0-ry)/len,(r0-rr)/len,r0);
}

static inline float length(Vec3f v) {
	return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

void CCFlow::gradientMatch(cv::Mat test, cv::Mat reference, int maxcount,TransRot initial, TransRot initialRange ) {

	float bestMatchScore = 1000000000; // huge
	float worstMatchScore = 0; // small
	TransRot current = initial;
	TransRot last = current;
	TransRot bestMatch = current;
	TransRot step = initialRange;
	Vec4f prev=Vec4f(0,0,0,bestMatchScore);

	//Mat debug=Mat::ones(20*16,20*16,CV_32FC1);
	//debug = debug.ones();
	//fprintf(stderr,"initial, at %f %f %f\n",current[0],current[1],current[2]);
	while (step[0]>1./8. && maxcount-->0) {
		Vec4f grad = correlateGradient(test,reference,current);
		//fprintf(stderr,"gradient is %f %f %f %f \n",grad[0],grad[1],grad[2],grad[3]);

		Vec3f combined = Vec3f(prev[0]+grad[0],prev[1]+grad[1],prev[2]+grad[2]);

		if (grad[3]>worstMatchScore) worstMatchScore=grad[3];
		if (grad[3]>0 && grad[3]<bestMatchScore) {
			bestMatchScore = grad[3];
			bestMatch = current;
		}
		if ((grad[0]==0 && grad[1]==0 && grad[2]==0) || combined==Vec3f(0,0,0) || step[0]>4*initialRange[0]) {
			last = current = initial;
			current[0] += rng->gaussian(initialRange[0]);
			current[1] += rng->gaussian(initialRange[1]);
			current[2] += rng->gaussian(initialRange[2]);
			step = initialRange;
			prev=Vec4f(0,0,0,bestMatchScore);
			//fprintf(stderr,"failure, now at %f %f %f\n",current[0],current[1],current[2]);
		} else {
			combined *= 1.0/length(combined); // normalize

			float success = (grad[0]*prev[0]+grad[1]*prev[1]+grad[2]*prev[2]); // dot product
			if (success<0) step *= 1.0+(2*success/3);


			current[0] = last[0] + step[0] * combined[0];
			current[1] = last[1] + step[1] * combined[1];
			current[2] = last[2] + step[2] * combined[2];

			//line(debug,Point(16*(last[0]+10.),16*(last[1]+10.)),Point(16*(current[0]+10.),16*(current[1]+10.)),0.5+ success/2);
			last=current;

			//fprintf(stderr,"success, now at %f %f %f\n",current[0],current[1],current[2]);
			//fprintf(stderr,"step is now %f %f %f %f\n",step[0],step[1],step[2],(grad[0]*prev[0]+grad[1]*prev[1]+grad[2]*prev[2]));
			
			prev=grad;
		}
		//imshow("debug3",debug);
		//waitKey(0);
		iterations++;
	}
	translation = Vec2f(bestMatch[0],bestMatch[1]);
	rotation = bestMatch[2];
	best = bestMatchScore;
	worst = worstMatchScore;
}


#define subresbits 5
#define subres (1<<subresbits)
#define rotationres (1.0/8.0)
float CCFlow::correlate(cv::Mat reference, cv::Mat test, cv::Mat rotMatrix) {

	double * m = (double*)rotMatrix.data;
	float squareError=0;
	int   pixels=0,missed=0;

//Mat debug=test.clone()*0;
	for (int y=0;y<reference.rows;y++) {
		for (int x=0;x<reference.cols;x++) {
			int tx = (float)(subres*(m[0]*(float)x + m[1]*(float)y + m[2]));
			int ty = (float)(subres*(m[3]*(float)x + m[4]*(float)y + m[5]));
			// check whether source pixel is within outer boundaries
			if ((tx/subres)>=0-border[0] && (tx/subres)+1<test.cols+border[2] && (ty/subres)>=0-border[1] && (ty/subres)+1<test.rows+border[3]) {
				pixels++;
				Vec3i tmp = reference.at<Vec3b>(y,x);
				Vec3i a1 = test.at<Vec3b>(ty/subres,tx/subres);
				Vec3i a2 = test.at<Vec3b>(ty/subres,tx/subres +1);
				Vec3i b1 = test.at<Vec3b>(ty/subres +1,tx/subres);
				Vec3i b2 = test.at<Vec3b>(ty/subres +1,tx/subres +1);
				Vec3i a = ((subres-1)-(tx%subres))*a1 + (tx%subres)*a2;
				Vec3i b = ((subres-1)-(tx%subres))*b1 + (tx%subres)*b2;
				Vec3i c = (((subres-1)-(ty%subres))*a + (ty%subres)*b);
				c[0]>>=(2*subresbits);
				c[1]>>=(2*subresbits);
				c[2]>>=(2*subresbits);
				tmp -= c;
				//debug.at<Vec3b>(y,x) = Vec3b(tmp[0]*tmp[0],tmp[1]*tmp[1],tmp[2]*tmp[2]);
				//debug.at<Vec3b>(y,x) = c;

				squareError += tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2];
			} else {
				missed++;
				squareError += 32*32*3;
				//pixels++;
			}
		}
	}
	if (pixels<missed) return -1;
	//for (int t=0;t<missed;t++) {
	//	squareError*=(pixels/(pixels-missed));
	//}
#if 0
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
#endif

	return squareError/pixels;
}

cv::Vec4f CCFlow::correlateGradient(cv::Mat test, cv::Mat reference, TransRot position) {

	Mat rotMatrix = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),position[2],1.0f);
	rotMatrix.at<double>(0,2)+=position[0];
	rotMatrix.at<double>(1,2)+=position[1];
	Mat rotMatrix2 = getRotationMatrix2D(Point2f(test.cols/2.,test.rows/2.),position[2]+rotationres,1.0f);
	rotMatrix2.at<double>(0,2)+=position[0];
	rotMatrix2.at<double>(1,2)+=position[1];

	double * m = (double*)rotMatrix.data;
	double * m2 = (double*)rotMatrix2.data;
	float squareError0=0;
	float squareError1=0;
	float squareError2=0;
	float squareError3=0;
	int   pixels=0,missed=0;

//Mat debug=test.clone()*0;
	const int xlimits0 = subres*-border[0];
	const int xlimits1 = subres*(test.cols+border[2]);
	const int ylimits0 = subres*-border[1];
	const int ylimits1 = subres*(test.cols+border[3]);
	for (int x=0;x<reference.cols;x++) {
		for (int y=0;y<reference.rows;y++) {
			const int tx0 = ((float)subres*(m[0]*(float)x + m[1]*(float)y + m[2]));
			const int tx1 = tx0+1;
			const int tx2 = tx0;
			const int ty0 = ((float)subres*(m[3]*(float)x + m[4]*(float)y + m[5]));
			const int ty1 = ty0;
			const int ty2 = ty0+1;
			const int tx3 = ((float)subres*(m2[0]*(float)x + m2[1]*(float)y + m2[2]));
			const int ty3 = ((float)subres*(m2[3]*(float)x + m2[4]*(float)y + m2[5]));
			// check whether source pixel is within outer boundaries
			if ( tx0>=xlimits0 && tx1<xlimits1 && ty0>=ylimits0 && ty2<ylimits1
			     && tx3>=xlimits0 && tx3<xlimits1 && ty3>=ylimits0 && ty3<ylimits1) {
				pixels++;
				Vec3i tmp = reference.at<Vec3b>(y,x);

				// its such a shame that most of these will be identical almost all the time,
				// but how could we check that fast? we will have to traverse pixel boundaries!
				Vec3i a01 = test.at<Vec3b>(ty0/subres,tx0/subres);
				Vec3i a02 = test.at<Vec3b>(ty0/subres,tx0/subres +1);
				Vec3i b01 = test.at<Vec3b>(ty0/subres +1,tx0/subres);
				Vec3i b02 = test.at<Vec3b>(ty0/subres +1,tx0/subres +1);
				Vec3i a11 = test.at<Vec3b>(ty1/subres,tx1/subres);
				Vec3i a12 = test.at<Vec3b>(ty1/subres,tx1/subres +1);
				Vec3i b11 = test.at<Vec3b>(ty1/subres +1,tx1/subres);
				Vec3i b12 = test.at<Vec3b>(ty1/subres +1,tx1/subres +1);
				Vec3i a21 = test.at<Vec3b>(ty2/subres,tx2/subres);
				Vec3i a22 = test.at<Vec3b>(ty2/subres,tx2/subres +1);
				Vec3i b21 = test.at<Vec3b>(ty2/subres +1,tx2/subres);
				Vec3i b22 = test.at<Vec3b>(ty2/subres +1,tx2/subres +1);
				Vec3i a31 = test.at<Vec3b>(ty3/subres,tx3/subres);
				Vec3i a32 = test.at<Vec3b>(ty3/subres,tx3/subres +1);
				Vec3i b31 = test.at<Vec3b>(ty3/subres +1,tx3/subres);
				Vec3i b32 = test.at<Vec3b>(ty3/subres +1,tx3/subres +1);

				// these however will always differ
				Vec3i a0 = ((subres-1)-(tx0%subres))*a01 + (tx0%subres)*a02;
				Vec3i b0 = ((subres-1)-(tx0%subres))*b01 + (tx0%subres)*b02;
				Vec3i c0 = (((subres-1)-(ty0%subres))*a0 + (ty0%subres)*b0);
				Vec3i a1 = ((subres-1)-(tx1%subres))*a11 + (tx1%subres)*a12;
				Vec3i b1 = ((subres-1)-(tx1%subres))*b11 + (tx1%subres)*b12;
				Vec3i c1 = (((subres-1)-(ty0%subres))*a1 + (ty0%subres)*b1);
				Vec3i a2 = ((subres-1)-(tx0%subres))*a21 + (tx0%subres)*a22;
				Vec3i b2 = ((subres-1)-(tx0%subres))*b21 + (tx0%subres)*b22;
				Vec3i c2 = (((subres-1)-(ty2%subres))*a2 + (ty2%subres)*b2);
				Vec3i a3 = ((subres-1)-(tx3%subres))*a31 + (tx3%subres)*a32;
				Vec3i b3 = ((subres-1)-(tx3%subres))*b31 + (tx3%subres)*b32;
				Vec3i c3 = (((subres-1)-(ty3%subres))*a3 + (ty3%subres)*b3);
				c0[0]>>=(2*subresbits);
				c0[1]>>=(2*subresbits);
				c0[2]>>=(2*subresbits);
				c1[0]>>=(2*subresbits);
				c1[1]>>=(2*subresbits);
				c1[2]>>=(2*subresbits);
				c2[0]>>=(2*subresbits);
				c2[1]>>=(2*subresbits);
				c2[2]>>=(2*subresbits);
				c3[0]>>=(2*subresbits);
				c3[1]>>=(2*subresbits);
				c3[2]>>=(2*subresbits);

				Vec3i diff0 = tmp - c0;
				Vec3i diff1 = tmp - c1;
				Vec3i diff2 = tmp - c2;
				Vec3i diff3 = tmp - c3;
				//debug.at<Vec3b>(y,x) = Vec3b(tmp[0]*tmp[0],tmp[1]*tmp[1],tmp[2]*tmp[2]);
				//debug.at<Vec3b>(y,x) = c;

				squareError0 += diff0[0]*diff0[0] + diff0[1]*diff0[1] + diff0[2]*diff0[2];
				squareError1 += diff1[0]*diff1[0] + diff1[1]*diff1[1] + diff1[2]*diff1[2];
				squareError2 += diff2[0]*diff2[0] + diff2[1]*diff2[1] + diff2[2]*diff2[2];
				squareError3 += diff3[0]*diff3[0] + diff3[1]*diff3[1] + diff3[2]*diff3[2];
			} else {
				missed++;
				squareError0 += 32*32*3;
				squareError1 += 32*32*3;
				squareError2 += 32*32*3;
				squareError3 += 32*32*3;
				//pixels++;
			}
		}
	}
	if (pixels<missed) return Vec4f(0,0,0,-1);

	Vec3f se(squareError0-squareError1,squareError0-squareError2,squareError0-squareError3);
	se *= (1./pixels);
	Vec3f se2(se[0]*se[0],se[1]*se[1],se[2]*se[2]);
	float len = sqrt(se2[0]+se2[1]+se2[2]);

	return Vec4f(se[0]/len,se[1]/len,se[2]/len,squareError0/pixels);
}



/* End of file. */
