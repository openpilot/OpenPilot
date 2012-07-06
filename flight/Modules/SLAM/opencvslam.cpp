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

#include "opencvslam.hpp"
#include "pyramidsenhanced.hpp"

// Private constants
#define DEG2RAD (3.1415926535897932/180.0)

using namespace cv;

OpenCVslam::OpenCVslam(SLAMSettingsData * newsettings) {

	settings = newsettings;
	frame    = 0;
	VideoSource  = NULL;
	currentFrame = NULL;
	lastFrame    = NULL;
}




void OpenCVslam::shrinkAndEnhance(const Mat& src, Mat& dst) {

	// Gauss kernel is
	// 1 4 6 4 1
	
	// Laplacian kernel is
	// -4-0 8 0 -4
	
	// target kernel is
	// -3 4 14 4 -3
	// or with double laplace
	// -7 4 22 4 -7
	PyrDownEnhanced enhanced;
	enhanced.pyrDownEnhanced(src,dst,-3,4,14);
	
}


void OpenCVslam::run() {

	Mat current[5]; // "pyramid" of current frame
	Mat last[5];    // "pyramid" of previous frame

	/* Initialize OpenCV */
	//VideoSource = NULL; //cvCaptureFromFile("test.avi");
	VideoSource = cvCaptureFromFile("test.avi");
	//VideoSource = cvCaptureFromCAM(0);
	//CvVideoWriter *VideoDest = cvCreateVideoWriter("output.avi",CV_FOURCC('F','M','P','4'),settings->FrameRate,cvSize(640,480),1);

	if (VideoSource) {
		cvGrabFrame(VideoSource);
		currentFrame = cvRetrieveFrame(VideoSource, 0);
		cvSetCaptureProperty(VideoSource, CV_CAP_PROP_FRAME_WIDTH,  settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]);
		cvSetCaptureProperty(VideoSource, CV_CAP_PROP_FRAME_HEIGHT, settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_Y]);
	}

	Mat last,current;
	Mat flow; // = Mat(480,640,CV_32FC1);
	Mat sflow[3];
	Mat cflow;


	//pyrDown(Mat(currentFrame),current);
	if (currentFrame) lastFrame = cvCloneImage(currentFrame);
	Mat(currentFrame).convertTo(current, CV_32F,1/256);
	//pyrDown(Mat(currentFrame),current);
	//pyrDown(current,current);
	last=current;

shrinkAndEnhance(current,flow);

	// debug output
	cvNamedWindow("debug",CV_WINDOW_AUTOSIZE);
	
	// synchronization delay, wait for attitude data - any attitude data
	// this is an evil hack but necessary for tests with log data to synchronize video and telemetry
	AttitudeActualGet(&attitudeActual);
	attitudeActual.Pitch=100;
	AttitudeActualSet(&attitudeActual);
	cvShowImage("debug",lastFrame);
	cvWaitKey(1);
	while (attitudeActual.Pitch==100) AttitudeActualGet(&attitudeActual);


	uint32_t timeval = PIOS_DELAY_GetRaw();
	uint32_t writeval = timeval;
	int32_t increment = floor(1000000./settings->FrameRate);
	int32_t writeincrement = increment;
	int32_t extraincrement=0;
	int32_t writeextra=0;
	fprintf(stderr,"init at %i increment is %i at %f fps\n",timeval, increment,settings->FrameRate);


	// Main task loop
	double oldpos=0;
	while (1) {
		frame++;
		cvWaitKey(1);
		if (VideoSource) {
			double pos=cvGetCaptureProperty(VideoSource, CV_CAP_PROP_POS_MSEC);
			increment=floor((pos-oldpos)*1000.);
			oldpos = pos;
		}
		if (extraincrement<-(increment+1000)) extraincrement=-(increment+1000);

		while (	(PIOS_DELAY_DiffuS(timeval)<(uint32_t)increment+extraincrement+1000)) {

			if (PIOS_DELAY_DiffuS(writeval)>=(uint32_t)writeincrement+writeextra+1000) {
				writeextra = (writeincrement+writeextra)-PIOS_DELAY_DiffuS(writeval);
				writeval = PIOS_DELAY_GetRaw();
				if (writeincrement+writeextra+1000<0) {
					writeextra=-(writeincrement+1000);
				}
				//struct writeframestruct x={VideoDest,lastFrame};
				//backgroundWriteFrame(x);
				//fprintf(stderr,".");
			} else {
				vTaskDelay(1/portTICK_RATE_MS);
			}
		}
			if (PIOS_DELAY_DiffuS(writeval)>=(uint32_t)writeincrement+writeextra+1000) {
				writeextra = (writeincrement+writeextra)-PIOS_DELAY_DiffuS(writeval);
				writeval = PIOS_DELAY_GetRaw();
				if (writeincrement+writeextra+1000<0) {
					writeextra=-(writeincrement+1000);
				}

				//struct writeframestruct x={VideoDest,lastFrame};
				//backgroundWriteFrame(x);
				//fprintf(stderr,".");
			}

		float dT = PIOS_DELAY_DiffuS(timeval) * 1.0e-6f;
		extraincrement = (increment+extraincrement)-PIOS_DELAY_DiffuS(timeval);
		timeval = PIOS_DELAY_GetRaw();

		// Grab the current camera image
		if (VideoSource) {
			// frame grabbing must take place outside of FreeRTOS scheduler,
			// since OpenCV's hardware IO does not like being interrupted.
			backgroundGrabFrame(VideoSource);
		}
		
		// Get the object data
		AttitudeActualGet(&attitudeActual);
		PositionActualGet(&positionActual);
		VelocityActualGet(&velocityActual);

		if (VideoSource) currentFrame = cvRetrieveFrame(VideoSource,0);

		if (currentFrame) {
			last=current;
			current=Mat(currentFrame);
			//Mat(currentFrame).convertTo(current, CV_32F,1./256.);
shrinkAndEnhance(current,flow);
//shrinkAndEnhance(flow,flow);
//shrinkAndEnhance(flow,flow);
//shrinkAndEnhance(flow,flow);
//shrinkAndEnhance(flow,flow);
			//pyrDown(Mat(currentFrame),current);
			//pyrDown(current,current);
		}
		if (currentFrame && lastFrame) {
			//Mat x1,x2;
			//cvtColor(last,x1,CV_RGB2GRAY);
			//cvtColor(current,x2,CV_RGB2GRAY);
			//flow=x2;
			//fprintf(stderr,"calculate flow...");
			//vPortEnterCritical();
			//calcOpticalFlowFarneback(x1,x2, flow, 0.5, 3, 9, 9, 5, 1.1, 0);
			//calcOpticalFlowFarneback(x1,x2, flow, 0.5, 4, 13, 1, 5, 1.1, 0);
			//calcOpticalFlowCorvus(last,curent,flow);
			//vPortExitCritical();
			//fprintf(stderr,"done\n");
			//split(flow,sflow);
			//sflow[2]=sflow[1];
			//merge(sflow,3,cflow);
			fprintf(stderr,".");
		}
		if (lastFrame) {
			cvReleaseImage(&lastFrame);
		}
		if (currentFrame) {
			lastFrame = cvCloneImage(currentFrame);


			// draw a line in the video coresponding to artificial horizon (roll+pitch)
			CvPoint center = cvPoint(
				  (settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]/2)
				  + (attitudeActual.Pitch * settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]/50.)
				  *sin(DEG2RAD*(attitudeActual.Roll))
				,
				  (settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_Y]/2) // -200
				  + (attitudeActual.Pitch * settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]/50.)
				  *cos(DEG2RAD*(attitudeActual.Roll))
				);
			// i want overloaded operands damnit!
			CvPoint right = cvPoint(
				  settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]*cos(DEG2RAD*(attitudeActual.Roll))/3
				,
				  -settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]*sin(DEG2RAD*(attitudeActual.Roll))/3
				);
			CvPoint left = cvPoint(center.x-right.x,center.y-right.y);
			right.x += center.x;
			right.y += center.y;
			//cvLine(lastFrame,left,right,CV_RGB(255,255,0),3,8,0);

			IplImage xflow = flow;
			cvShowImage("debug",&xflow);
		cvWaitKey(1);
		}
	


		//fprintf(stderr,"frame %i took %f ms (%f fps)\n",frame,dT,1./dT);
	}
}


/**
 * bridge to C code
 */
void opencvslam(SLAMSettingsData *settings) {
	OpenCVslam *slam = new OpenCVslam(settings);
	slam->run();
}

/**
 * @}
 * @}
 */
