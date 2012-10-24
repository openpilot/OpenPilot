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
#include "rtslam.h"

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





void OpenCVslam::run() {

	/* Initialize RTSlam */
	rtslam = new RTSlam(4,15);
	//rtslam = new RTSlam(0,30);

	rtslam->init();

	/* Initialize OpenCV */
	//VideoSource = NULL; //cvCaptureFromFile("test.avi");
	VideoSource = cvCaptureFromFile("test.avi");
	//VideoSource = cvCaptureFromCAM(0);
	
	VideoDest = NULL;
	//CvVideoWriter *VideoDest = cvCreateVideoWriter("output.avi",CV_FOURCC('F','M','P','4'),settings->FrameRate,cvSize(640,480),1);

	if (VideoSource) {
		cvSetCaptureProperty(VideoSource, CV_CAP_PROP_FRAME_WIDTH,  settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_X]);
		cvSetCaptureProperty(VideoSource, CV_CAP_PROP_FRAME_HEIGHT, settings->FrameDimensions[SLAMSETTINGS_FRAMEDIMENSIONS_Y]);
		cvGrabFrame(VideoSource);
		currentFrame = cvRetrieveFrame(VideoSource, 0);
	}

	// debug output
	cvNamedWindow("debug",CV_WINDOW_AUTOSIZE);
	
	// synchronization delay, wait for attitude data - any attitude data
	// this is an evil hack but necessary for tests with log data to synchronize video and telemetry
	AttitudeActualGet(&attitudeActual);
	attitudeActual.Pitch=100;
	AttitudeActualSet(&attitudeActual);
	cvShowImage("debug",currentFrame);
	//cvWaitKey(1);
	while (attitudeActual.Pitch==100) AttitudeActualGet(&attitudeActual);


	uint32_t timeval = PIOS_DELAY_GetRaw();
	uint32_t writeval = timeval;
	int32_t increment = floor(1000000./settings->FrameRate);
	int32_t writeincrement = increment;
	int32_t extraincrement=0;
	int32_t writeextra=0;
	fprintf(stderr,"init at %i increment is %i at %f fps\n",timeval, increment,settings->FrameRate);

	/** start RTSLAM in its own thread (unknown to freertos since rtslam internally uses boost for thread control) **/
	backgroundrtslam(rtslam);

	// Main task loop
	double oldpos=0;

	// debugging iterations
	int iterations;
	while (1) {
		frame++;
		//backgroundWaitKey(1);
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
				if (VideoDest) {
					struct writeframestruct x={VideoDest,lastFrame};
					if (lastFrame) backgroundWriteFrame(x);
					fprintf(stderr,".");
				}
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

				if (VideoDest) {
					struct writeframestruct x={VideoDest,lastFrame};
					if (lastFrame) backgroundWriteFrame(x);
					fprintf(stderr,".");
				}
			}

		float dT = PIOS_DELAY_DiffuS(timeval) * 1.0e-6f;
		extraincrement = (increment+extraincrement)-PIOS_DELAY_DiffuS(timeval);
		timeval = PIOS_DELAY_GetRaw();

		// Get the object data
		AttitudeActualGet(&attitudeActual);
		PositionActualGet(&positionActual);
		VelocityActualGet(&velocityActual);
		hardware::OpenPilotStateInformation state;
		state.position.x = positionActual.North/100.;
		state.position.y = -positionActual.East/100.;
		state.position.z = -positionActual.Down/100.;
		//state.position.x = 0; // +x
		//state.position.y = 0; // -y
		//state.position.z = .1; // -z
		state.attitude.roll=attitudeActual.Roll * DEG2RAD;
		state.attitude.pitch=-attitudeActual.Pitch * DEG2RAD;
		state.attitude.yaw=-attitudeActual.Yaw * DEG2RAD;
		//state.attitude.roll=0*DEG2RAD; // +roll
		//state.attitude.pitch=-0*DEG2RAD; // -pitch
		//state.attitude.yaw=-0*DEG2RAD; // -yaw
		state.positionVariance = -1;
		state.attitudeVariance = -1;
		//state.positionVariance = powf(0.1,2);
		//state.attitudeVariance = powf(0.1*DEG2RAD,2);
		rtslam->state(&state);

		// Grab the current camera image
		if (VideoSource) {
			// frame grabbing must take place outside of FreeRTOS scheduler,
			// since OpenCV's hardware IO does not like being interrupted.
			backgroundGrabFrame(VideoSource);
		}
		
		//if (VideoSource) currentFrame = cvRetrieveFrame(VideoSource,0);
		if (VideoSource) currentFrame = backgroundRetrieveFrame(VideoSource);

		if (lastFrame) {
			cvReleaseImage(&lastFrame);
		}
		if (currentFrame) {
			if (frame>=4) {
				rtslam->videoFrame(currentFrame);
			}
			lastFrame = cvCloneImage(currentFrame);
			/*
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
			cvLine(lastFrame,left,right,CV_RGB(255,255,0),3,8,0);
			*/
			// draw coordinate system
			float q1[4]={attitudeActual.q1,attitudeActual.q2,attitudeActual.q3,attitudeActual.q4};
			fprintf(stderr," %f  %f  %f  %f \n",q1[0],q1[1],q1[2],q1[3]);
			float q2[4]={0,0,0,0};
			float q[4]={0,0,0,0};
			float tilt[3]={4,-12,-3};
			RPY2Quaternion(tilt,q2);
			quat_mult(q1,q2,q);
			float Rbe[3][3];
			Quaternion2R(q,Rbe);
			Mat rotation(3,3,CV_64F);
			for (int a=0;a<3;a++) {
				for (int b=0;b<3;b++) {
					rotation.at<double>(a,b)=Rbe[a][b];
				}
			}
			
			Mat translation(Mat::zeros(3,4,CV_64F));
			translation.at<double>(0,0)=1.0;
			translation.at<double>(1,1)=1.0;
			translation.at<double>(2,2)=1.0;
			translation.at<double>(0,3)=-positionActual.North;
			translation.at<double>(1,3)=-positionActual.East;
			translation.at<double>(2,3)=-positionActual.Down;
			fprintf(stderr," %f  %f  %f\n",(double)positionActual.North,(double)positionActual.East,(double)positionActual.Down);
			
			Mat correction(Mat::zeros(3,3,CV_64F));
			correction.at<double>(0,1)=1;
			correction.at<double>(1,2)=1;
			correction.at<double>(2,0)=1;

			Mat camera(correction*rotation*translation);
			//camera.diag() = Mat::ones(3,1,CV_64F);
			Draw3d d(Vec2i(320,240),Vec2i(1015,1015),Vec3d(0.0,0.0,0),camera);
			Mat lf(lastFrame);
			Scalar c1(0,255,0);
			Scalar c11(255,255,0);
			Scalar c2(0,255,255);
			Scalar c3(0,64,255);
			// z plane
			vPortEnterCritical();
			d.drawGrid(lf,geo::Plane(Point3d(-1e5,-1e5,4000),Point3d(1e5,-1e5,4000),Point3d(-1e5,1e5,4000)),5e3,c1,1,8,0);
			d.drawGrid(lf,geo::Plane(Point3d(-1e7,-1e7,4000),Point3d(1e7,-1e7,4000),Point3d(-1e7,1e7,4000)),1e6,c1,1,8,0);
			//d.drawGrid(lf,geo::Plane(Point3d(-100000,-100000,10000),Point3d(100000,-100000,0),Point3d(-100000,100000,-10000)),5000,c11,1,8,0);
			// xplane
			//d.drawGrid(lf,geo::Plane(Point3d(-100000,-100000,-1000),Point3d(-100000,100000,-1000),Point3d(-100000,-100000,1000)),5000,c2,1,8,0);
			//d.drawGrid(lf,geo::Plane(Point3d(100000,-100000,-100000),Point3d(100000,100000,-100000),Point3d(-10000,-100000,100000)),5000,c2,1,8,0);
			// yplane
			//d.drawGrid(lf,geo::Plane(Point3d(-100000,-100000,-1000),Point3d(100000,-100000,-1000),Point3d(-100000,-100000,1000)),5000,c3,1,8,0);
			//d.drawGrid(lf,geo::Plane(Point3d(-100000,100000,-100000),Point3d(100000,100000,-100000),Point3d(-100000,100000,100000)),5000,c3,1,8,0);
			cvShowImage("debug",lastFrame);
			vPortExitCritical();
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
