/**
 * \file HardwareSensorCameraUeye.cpp
 * \date 18/06/2010
 * \author croussil
 * \ingroup rtslam
 */

#include <algorithm>
#include <sstream>
#include <fstream>

#if 0
// creates conflict with boost sandbox with boost 1.42 in debug
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#endif

#include "kernel/timingTools.hpp"
#include "rtslam/hardwareSensorCameraUeye.hpp"


#include <image/Image.hpp>

#ifdef HAVE_UEYE
/// convert UEYETIME to double epoch
double convertUeyeTime(UEYETIME &ueyeTime)
{
	struct tm t;
	t.tm_year = ueyeTime.wYear-1900;
	t.tm_mon = ueyeTime.wMonth-1;
	t.tm_mday = ueyeTime.wDay;
	t.tm_hour = ueyeTime.wHour;
	t.tm_min = ueyeTime.wMinute;
	t.tm_sec = ueyeTime.wSecond;
	t.tm_isdst = -1;
	return mktime(&t) + ueyeTime.wMilliseconds * 1e-3;
}
#endif

namespace jafar {
namespace rtslam {
namespace hardware {

	void HardwareSensorCameraUeye::preloadTask(void)
	{ try {
		char *image;
		int imageID;
		UEYEIMAGEINFO imageInfo;

		while(true)
		{
			// acquire the image
#ifdef HAVE_UEYE
				int buff_write = getWritePos();

				if (is_WaitForNextImage(camera, 1000, &image, &imageID) != IS_SUCCESS) continue;
				bufferSpecPtr[buff_write]->arrival = kernel::Clock::getTime();
				// TODO would be better if avoiding copy here
				memcpy(bufferImage[buff_write]->imageData, image, bufferImage[buff_write]->imageSize);
				if (is_UnlockSeqBuf(camera, imageID, image) != IS_SUCCESS)  { std::cerr << "HardwareSensorCameraUeye: unlock failed" << std::endl; }

				if (is_GetImageInfo(camera, imageID, &imageInfo, sizeof(imageInfo)) != IS_SUCCESS) continue;
				bufferSpecPtr[buff_write]->timestamp = convertUeyeTime(imageInfo.TimestampSystem);
				// TODO use camera timestamp to filter pc timestamp
				//double cameraTimeStamp = imageInfo.u64TimestampDevice * 1e-7;
				//printf("%.16g\t%.19g\t%.12g\n", bufferSpecPtr[buff_write]->timestamp, bufferSpecPtr[buff_write]->arrival, imageInfo.u64TimestampDevice * 1e-7);
				last_timestamp = bufferSpecPtr[buff_write]->timestamp;
#endif
			incWritePos();
			condition.setAndNotify(1);
		}
	} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }

	
	
	void HardwareSensorCameraUeye::init(int mode, std::string dump_path, cv::Size imgSize)
	{
		this->mode = mode;
		HardwareSensorCamera::init(dump_path, imgSize);

		// start save tasks
		if (mode == 1)
		{
			saveTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraUeye::saveTask,this));
			savePushTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraUeye::savePushTask,this));
		}
	}
	
	void HardwareSensorCameraUeye::start()
	{
		// start acquire task
		if (started) { std::cout << "Warning: This HardwareSensorCameraUeye has already been started" << std::endl; return; }
		started = true;
		last_timestamp = kernel::Clock::getTime();
		if (mode == 2)
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraUeye::preloadTaskOffline,this));
		else
			preloadTask_thread = new boost::thread(boost::bind(&HardwareSensorCameraUeye::preloadTask,this));
	}
		
	
	HardwareSensorCameraUeye::HardwareSensorCameraUeye(kernel::VariableCondition<int> &condition, cv::Size imgSize, std::string dump_path):
		HardwareSensorCamera(condition, imgSize, dump_path)
	{}
	

#ifdef HAVE_UEYE
	void HardwareSensorCameraUeye::init(const std::string &camera_id, cv::Size imgSize, double shutter, double freq, int trigger, int mode, std::string dump_path)
	{
		int r;
		char *msg;
		// configure camera
		if (mode == 0 || mode == 1)
		{
			// configure
			int ncameras;
			if (is_GetNumberOfCameras(&ncameras) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_GetNumberOfCameras: " << msg << std::endl; return; }
			if (ncameras <= 0) { std::cerr << "HardwareSensorCameraUeye::init: Could not find camera" << std::endl; return; }
			PUEYE_CAMERA_LIST camList = (PUEYE_CAMERA_LIST) new char[sizeof(DWORD) + ncameras * sizeof(UEYE_CAMERA_INFO)];
			camList->dwCount = ncameras;
			if (is_GetCameraList(camList) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_GetCameraList: " << msg << std::endl; return; }
			camera = 0; // use first camera
			if (is_InitCamera(&camera, NULL) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_InitCamera: " << msg << std::endl; return; }
			SENSORINFO sensorInfo;
			if (is_GetSensorInfo(camera, &sensorInfo) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_GetSensorInfo: " << msg << std::endl; return; }
			std::cout << "HardwareSensorCameraUeye: Using sensor " << sensorInfo.strSensorName << " ID " << camList->uci[camera].SerNo << std::endl;
			if (sensorInfo.bGlobShutter == FALSE) { std::cerr << "HardwareSensorCameraUeye: Cannot use a rolling shutter camera" << std::endl; return; }
			IS_RECT rect;
			rect.s32X = (sensorInfo.nMaxWidth-imgSize.width)/2; rect.s32Width = imgSize.width;
			rect.s32Y = (sensorInfo.nMaxHeight-imgSize.height)/2; rect.s32Height = imgSize.height;
			if (is_AOI(camera, IS_AOI_IMAGE_SET_AOI, (void*)&rect, sizeof(IS_RECT)) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_AOI(IS_AOI_IMAGE_SET_AOI): " << msg << std::endl; return; }
			if (is_SetColorMode(camera, IS_CM_MONO8 ) != IS_SUCCESS )
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetColorMode(IS_CM_MONO8): " << msg << std::endl; return; }
			if (is_SetFrameRate(camera, freq, &realFreq) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetFrameRate: " << msg << std::endl; return; }
			std::cout << "Camera set to freq " << realFreq << " Hz (external trigger " << trigger << ")" << std::endl;
			if (shutter > 1e-6)
			{
				shutter *= 1000; // convert to ms
				if (is_SetAutoParameter(camera, IS_SET_AUTO_SHUTTER_MAX, &shutter, NULL) != IS_SUCCESS) // must be before ENABLE_AUTO_SHUTTER
					{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetAutoParameter(IS_SET_AUTO_SHUTTER_MAX): " << msg << std::endl; return; }
			}
			double exposure = 1;
			if (is_SetAutoParameter(camera, IS_SET_ENABLE_AUTO_SHUTTER, &exposure, NULL) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetAutoParameter(IS_SET_ENABLE_AUTO_SHUTTER): " << msg << std::endl; return; }
			if (is_SetAutoParameter(camera, IS_SET_ENABLE_AUTO_GAIN, &exposure, NULL) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetAutoParameter(IS_SET_ENABLE_AUTO_GAIN): " << msg << std::endl; return; }
			// TODO set trigger
			if (is_SetExternalTrigger(camera, IS_SET_TRIGGER_OFF) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_SetExternalTrigger: " << msg << std::endl; return; }

			for (int i = 0; i < 2; ++i)
			{
				char* imageMem;
				int pid;

				if (is_AllocImageMem(camera, imgSize.width, imgSize.height, 8, &imageMem, &pid) != IS_SUCCESS)
					{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_AllocImageMem: " << msg << std::endl; return; }
				if( is_AddToSequence(camera, imageMem, pid) != IS_SUCCESS)
					{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_AddToSequence: " << msg << std::endl; return; }
			}

			//int widthStep;
			//if (is_GetImageMemPitch(camera, &widthStep) != IS_SUCCESS) { std::cerr << "HardwareSensorCameraUeye: Could not get widthStep" << std::endl; return; }
			//if (widthStep != bufferImage[0]->widthStep) { std::cerr << "HardwareSensorCameraUeye: uEye widthStep " << widthStep << " != opencv widthStep " << bufferImage[0]->widthStep << std::endl; return; }
			if (is_InitImageQueue (camera, 0) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_InitImageQueue: " << msg << std::endl; return; }
			if (is_CaptureVideo(camera, IS_DONT_WAIT) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::init, is_CaptureVideo: " << msg << std::endl; return; }
		}

		init(mode, dump_path, imgSize);
	}

	HardwareSensorCameraUeye::HardwareSensorCameraUeye(kernel::VariableCondition<int> &condition, int bufferSize, const std::string &camera_id, cv::Size size, double freq, int trigger, double shutter, int mode, std::string dump_path):
		HardwareSensorCamera(condition, bufferSize)
	{
		init(camera_id, size, shutter, freq, trigger, mode, dump_path);
	}
#endif


	HardwareSensorCameraUeye::~HardwareSensorCameraUeye()
	{
		int r;
		char *msg;
#ifdef HAVE_UEYE
		if (mode == 0 || mode == 1)
		{
			if (is_StopLiveVideo(camera, IS_WAIT) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::~, is_StopLiveVideo: " << msg << std::endl; }
			if (is_ExitImageQueue(camera) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::~, is_ExitImageQueue: " << msg << std::endl; }
			if (is_ClearSequence(camera) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::~, is_ClearSequence: " << msg << std::endl; }
			// TODO get back buffers to release them, or store them at creation
			//for (int i = 0; i < 2; ++i) is_FreeImageMem(camera, , );
			if (is_ExitCamera(camera) != IS_SUCCESS)
				{ is_GetError (camera, &r, &msg); std::cerr << "HardwareSensorCameraUeye::~, is_ExitCamera: " << msg << std::endl; }
		}
		saveTask_cond.wait(boost::lambda::_1 == 0);
#endif
	}


}}}

