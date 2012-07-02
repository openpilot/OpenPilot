/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       backgroundio.c
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

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "backgroundio.h"
#include "openpilot.h"
#include "signal.h"
#include "pthread.h"

static void* grabThread(void* arg) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	cvGrabFrame((CvCapture*)arg);	
	return NULL;
}

static void* waitkeyThread(void* arg) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	cvWaitKey(*((uint32_t*)arg));
	return NULL;
}

static void* retrieveThread(void* arg) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	return cvRetrieveFrame((CvCapture*)arg,0);	
}

static void* writeThread(void* arg) {
	sigset_t set;
	sigfillset(&set);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
	cvWriteFrame(((struct writeframestruct*)arg)->VideoDest,((struct writeframestruct*)arg)->frame);
	return NULL;
}

void backgroundGrabFrame(CvCapture *VideoSource) {
	pthread_t grabber;
	pthread_attr_t threadAttributes;
	pthread_attr_init( &threadAttributes );
	pthread_attr_setdetachstate( &threadAttributes, PTHREAD_CREATE_JOINABLE );
	pthread_create( &grabber, &threadAttributes,
                          &grabThread, VideoSource);
	pthread_join(grabber,NULL);
}

void backgroundWaitKey(uint32_t ms) {
	pthread_t waitkeyer;
	pthread_attr_t threadAttributes;
	pthread_attr_init( &threadAttributes );
	pthread_attr_setdetachstate( &threadAttributes, PTHREAD_CREATE_JOINABLE );
	pthread_create( &waitkeyer, &threadAttributes,
                          &waitkeyThread, &ms);
	pthread_join(waitkeyer,NULL);

}

IplImage* backgroundRetrieveFrame(CvCapture *VideoSource) {
	IplImage* result;
	pthread_t retriever;
	pthread_attr_t threadAttributes;
	pthread_attr_init( &threadAttributes );
	pthread_attr_setdetachstate( &threadAttributes, PTHREAD_CREATE_JOINABLE );
	pthread_create( &retriever, &threadAttributes,
                          &retrieveThread, VideoSource);
	pthread_join(retriever,(void**)&result);
	return result;
}

void backgroundWriteFrame(struct writeframestruct info) {
	pthread_t writer;
	pthread_attr_t threadAttributes;
	pthread_attr_init( &threadAttributes );
	pthread_attr_setdetachstate( &threadAttributes, PTHREAD_CREATE_JOINABLE );
	pthread_create( &writer, &threadAttributes,
                          &writeThread, &info);
	pthread_join(writer,NULL);
}


/**
 * @}
 * @}
 */
