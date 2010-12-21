/**
 * \file demo_slam.cpp
 *
 * ## Add brief description here ##
 *
 * \author jsola@laas.fr
 * \date 28/04/2010
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include <iostream>
#include <boost/shared_ptr.hpp>
//#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <time.h>
#include <map>
#include <getopt.h>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "kernel/timingTools.hpp"
#include "kernel/dataLog.hpp"
#include "jmath/random.hpp"
#include "jmath/matlab.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/rawProcessors.hpp"
//#include "rtslam/robotOdometry.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/robotInertial.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
//#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationMakers.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/dataManagerOnePointRansac.hpp"

#include "rtslam/hardwareSensorCameraFirewire.hpp"
#include "rtslam/hardwareEstimatorMti.hpp"

#include "rtslam/display_qt.hpp"
#include "rtslam/display_gdhe.hpp"

#include "rtslam/simuRawProcessors.hpp"
#include "rtslam/hardwareSensorAdhocSimulator.hpp"

using namespace jblas;
using namespace jafar;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;

jblas::vec sqrt(jblas::vec x)
{
	jblas::vec res(x.size());
	for(size_t i = 0; i < x.size(); ++i) res(i) = sqrt(x(i));
	return res;
}

typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinHole, LandmarkEuclideanPoint,
	 AppearanceImagePoint, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> PinholeEucpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinHole, LandmarkEuclideanPoint,
	 simu::AppearanceSimu, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> PinholeEucpSimuObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinHole, LandmarkAnchoredHomogeneousPoint,
	AppearanceImagePoint, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH> PinholeAhpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinHole, LandmarkAnchoredHomogeneousPoint,
	simu::AppearanceSimu, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH> PinholeAhpSimuObservationMaker;

typedef DataManagerOnePointRansac<RawImage, SensorPinHole, FeatureImagePoint, image::ConvexRoi, ActiveSearchGrid, ImagePointHarrisDetector, ImagePointZnccMatcher> DataManager_ImagePoint_Ransac;
typedef DataManagerOnePointRansac<simu::RawSimu, SensorPinHole, simu::FeatureSimu, image::ConvexRoi, ActiveSearchGrid, simu::DetectorSimu<image::ConvexRoi>, simu::MatcherSimu<image::ConvexRoi> > DataManager_ImagePoint_Ransac_Simu;

///##############################################

#define RANSAC 1
#define EVENT_BASED_RAW 1

int mode = 0;
time_t rseed;

///##############################################

enum { iDispQt = 0, iDispGdhe, iRenderAll, iReplay, iDump, iRandSeed, iPause, iLog, iVerbose, iRobot, iTrigger, iSimu, nIntOpts };
int intOpts[nIntOpts] = {0};
const int nFirstIntOpt = 0, nLastIntOpt = nIntOpts-1;

enum { fFreq = 0, fShutter, nFloatOpts };
double floatOpts[nFloatOpts] = {0.0};
const int nFirstFloatOpt = nIntOpts, nLastFloatOpt = nIntOpts+nFloatOpts-1;

enum { sSlamConfig = 0, sDataPath, nStrOpts };
std::string strOpts[nStrOpts];
const int nFirstStrOpt = nIntOpts+nFloatOpts, nLastStrOpt = nIntOpts+nFloatOpts+nStrOpts-1;

enum { bHelp = 0, bUsage, nBreakingOpts };
const int nFirstBreakingOpt = nIntOpts+nFloatOpts+nStrOpts, nLastBreakingOpt = nIntOpts+nFloatOpts+nStrOpts+nBreakingOpts-1;


struct option long_options[] = {
	// int options
	{"disp-2d", 2, 0, 0},
	{"disp-3d", 2, 0, 0},
	{"render-all", 2, 0, 0},
	{"replay", 2, 0, 0},
	{"dump", 2, 0, 0},
	{"rand-seed", 2, 0, 0},
	{"pause", 2, 0, 0},
	{"log", 2, 0, 0},
	{"verbose", 2, 0, 0},
	{"robot", 2, 0, 0}, // should be in config file
	{"trigger", 2, 0, 0}, // should be in config file
	{"simu", 2, 0, 0},
	// double options
	{"freq", 2, 0, 0}, // should be in config file
	{"shutter", 2, 0, 0}, // should be in config file
	// string options
	{"slam-config", 1, 0, 0},
	{"data-path", 1, 0, 0},
	// breaking options
	{"help",0,0,0},
	{"usage",0,0,0},
};

///##############################################

const int slam_priority = -20; // needs to be started as root to be < 0
const int display_priority = 10;
const int display_period = 100; // ms

///##############################################

const std::string MTI_DEVICE = "/dev/ttyUSB0";
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";

// time
const unsigned N_FRAMES = 500000;

// map size in # of states : 13 for robot plus 100 3D-landmarks
const unsigned MAP_SIZE = 500;

// constant velocity robot uncertainties and perturbations
const double UNCERT_VLIN = .5; // m/s
const double UNCERT_VANG = .5; // rad/s
const double PERT_VLIN = 2.0; // m/s per sqrt(s)
const double PERT_VANG = 2.0; // rad/s per sqrt(s)

// inertial robot initial uncertainties and perturbations - in addition to constant velocity option UNCERT_VLIN.
//if (intOpts[iRobot] == 1) // == robot inertial
const double UNCERT_GRAVITY = 1; // m/s^2
const double UNCERT_ABIAS = 0.05*17.0; // 5% of full scale
const double UNCERT_WBIAS = 0.05*jmath::degToRad(300.0); // 5% of full scale
const double PERT_AERR = 0.001*sqrt(30.0/100.0); // m/s per sqrt(s), IMU acc error (MTI = 0.001*sqrt(40Hz/100Hz))
const double PERT_WERR = jmath::degToRad(0.05)*sqrt(40.0/100.0); // rad per sqrt(s), IMU gyro error (MTI = 0.05*sqrt(30Hz/100Hz))
const double PERT_RANWALKACC = 0; // m/s^2 per sqrt(s), IMU a_bias random walk
const double PERT_RANWALKGYRO = 0; // rad/s^2 per sqrt(s), IMU w_bias random walk

// pin-hole:

// simu:
const unsigned IMG_WIDTH_SIMU = 640;
const unsigned IMG_HEIGHT_SIMU = 480;
const double INTRINSIC_SIMU[4] = { 320.0,   240.0,   500.0,   500.0 };
const double DISTORTION_SIMU[3] = { -0.25,   0.10, 0.0 };

// flea2 with original obj
/*const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 301.27013,   266.86136,   497.28243,   496.81116 };
const double DISTORTION[3] = { -0.23193,   0.11306, 0.0 }; //{-0.27965, 0.20059, -0.14215}; //{-0.27572, 0.28827};
*/

// flea2 with original obj after 2010/11/24
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 306.2969,   264.7741,   499.9177,   494.4829 };
const double DISTORTION[3] = { -0.2293129, 0.08793152, -0.01349877 };


// flea2 with Schneider lens
/*const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 3.043313e+02, 2.407406e+02, 6.714993e+02, 6.714853e+02};
const double DISTORTION[3] = { -2.900038e-01, 2.388370e-01, -2.006549e-01 };
*/

// jmcodol's robot
/*const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 327.53722,   222.40418,   533.18050,   531.56182 };
//const double DISTORTION[2] = { 0.08577,   -0.22006, 0.0 };
const double DISTORTION[3] = { 0.0, 0.0};
*/

/*// saragosse sequence
const unsigned IMG_WIDTH = 512;
const unsigned IMG_HEIGHT = 384;
const double INTRINSIC[4] = { 281.647017175628, 198.770772126498,   534.760390823972,   535.280428739968 };
const double DISTORTION[3] = { -0.27546592313146917, 0.12644899941674098, 0.036648747902512463 };
*/

//
const unsigned CORRECTION_SIZE = 4;
const double PIX_NOISE = 1.0;
const double PIX_NOISE_SIMUFACTOR = 1.0;

// lmk management
const double D_MIN = .5;
const double REPARAM_TH = 0.1;

// data manager: quick Harris detector
const unsigned HARRIS_CONV_SIZE = 5;
const double HARRIS_TH = 15.0;
const double HARRIS_EDDGE = 2.0;

const unsigned DESC_SIZE = 31;
const double DESC_SCALE_STEP = 2.0;
const double DESC_ANGLE_STEP = jmath::degToRad(10.0);
const DescriptorImagePointMultiView::PredictionType DESC_PREDICTION_TYPE = DescriptorImagePointMultiView::ptAffine;

// data manager: zncc matcher and one-point-ransac
const unsigned PATCH_SIZE = 13; // in pixels
const unsigned MAX_SEARCH_SIZE = 50000; // in number of pixels
const unsigned KILL_SEARCH_SIZE = 100000; // in number of pixels
const double MATCH_TH = 0.90;
const double MAHALANOBIS_TH = 3; // in n_sigmas
const unsigned N_UPDATES_TOTAL = 20;
const unsigned N_UPDATES_RANSAC = 15;
const unsigned N_INIT = 10;
const unsigned N_RECOMP_GAINS = 3;
const double RANSAC_LOW_INNOV = 1.0; // in pixels

#if RANSAC
const unsigned RANSAC_NTRIES = 6;
#else
const unsigned RANSAC_NTRIES = 0;
#endif
const double MIN_SCORE = 0.9;
const double PARTIAL_POSITION = 0.25;

// data manager: active search tesselation grid for new features detection
const unsigned GRID_VCELLS = 3;
const unsigned GRID_HCELLS = 3;
const unsigned GRID_MARGIN = 11;
const unsigned GRID_SEPAR = 20;


///##############################################



void demo_slam01_main(world_ptr_t *world) {
	
	vec intrinsic, distortion;
	int img_width, img_height;
	if (intOpts[iSimu] != 0)
	{
		img_width = IMG_WIDTH_SIMU;
		img_height = IMG_HEIGHT_SIMU;
		intrinsic = createVector<4> (INTRINSIC_SIMU);
		distortion = createVector<sizeof(DISTORTION_SIMU)/sizeof(double)> (DISTORTION);
		
	} else
	{
		img_width = IMG_WIDTH;
		img_height = IMG_HEIGHT;
		intrinsic = createVector<4> (INTRINSIC);
		distortion = createVector<sizeof(DISTORTION)/sizeof(double)> (DISTORTION);
	}
	
	
	
	
	boost::condition_variable rawdata_condition;
	boost::mutex rawdata_mutex;
	//boost::mutex rawdata_condmutex;
	boost::scoped_ptr<kernel::DataLogger> dataLogger;
	if (intOpts[iLog])
	{
		dataLogger.reset(new kernel::DataLogger(strOpts[sDataPath] + "/rtslam.log"));
		dataLogger->writeCurrentDate();
		dataLogger->writeNewLine();
		
		// write options to log
		std::ostringstream oss;
		for(int i = 0; i < nIntOpts; ++i)
			{ oss << long_options[i+nFirstIntOpt].name << " = " << intOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		for(int i = 0; i < nFloatOpts; ++i)
			{ oss << long_options[i+nFirstFloatOpt].name << " = " << floatOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		for(int i = 0; i < nStrOpts; ++i)
			{ oss << long_options[i+nFirstStrOpt].name << " = " << strOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		dataLogger->writeNewLine();
	}

	switch (intOpts[iVerbose])
	{
		case 0: debug::DebugStream::setLevel("rtslam", debug::DebugStream::Off); break;
		case 1: debug::DebugStream::setLevel("rtslam", debug::DebugStream::Trace); break;
		case 2: debug::DebugStream::setLevel("rtslam", debug::DebugStream::Warning); break;
		case 3: debug::DebugStream::setLevel("rtslam", debug::DebugStream::Debug); break;
		case 4: debug::DebugStream::setLevel("rtslam", debug::DebugStream::VerboseDebug); break;
		default: debug::DebugStream::setLevel("rtslam", debug::DebugStream::VeryVerboseDebug); break;
	}

	
	// pin-hole parameters in BOOST format
	boost::shared_ptr<ObservationFactory> obsFact(new ObservationFactory());
	if (intOpts[iSimu] != 0)
	{
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpSimuObservationMaker(REPARAM_TH, KILL_SEARCH_SIZE, 30, 0.5, 0.5, D_MIN, PATCH_SIZE)));
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpSimuObservationMaker(REPARAM_TH, KILL_SEARCH_SIZE, 30, 0.5, 0.5, D_MIN, PATCH_SIZE)));
	} else
	{
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker(REPARAM_TH, KILL_SEARCH_SIZE, 30, 0.5, 0.5, D_MIN, PATCH_SIZE)));
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker(REPARAM_TH, KILL_SEARCH_SIZE, 30, 0.5, 0.5, D_MIN, PATCH_SIZE)));
	}


	// ---------------------------------------------------------------------------
	// --- INIT ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : 1 map and map-manager, 2 robs, 3 sens and data-manager.
	world_ptr_t worldPtr = *world;
	//worldPtr->display_mutex.lock();


	// 1. Create maps.
	map_ptr_t mapPtr(new MapAbstract(MAP_SIZE));
	worldPtr->addMap(mapPtr);
	// 1b. Create map manager.
	boost::shared_ptr<MapManager<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> > mmPoint(new MapManager<
	    LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> ());
	mmPoint->linkToParentMap(mapPtr);

	boost::shared_ptr<simu::AdhocSimulator> simulator;
	if (intOpts[iSimu] != 0)
	{
		simulator.reset(new simu::AdhocSimulator());
		jblas::vec3 pose;
		
		const int maxnpoints = 1000;
		int npoints = 0;
		double points[maxnpoints][3];
		
		switch (intOpts[iSimu]/10)
		{
			case 1: {
				// regular grid
				const int npoints_ = 3*11*13; npoints = npoints_;
				for(int i = 0, z = -1; z <= 1; ++z) for(int y = -3; y <= 7; ++y) for(int x = -6; x <= 6; ++x, ++i)
					{ points[i][0] = x*1.0; points[i][1] = y*1.0; points[i][2] = z*1.0; }
				break;
			}
			case 2: {
				// flat square
				const int npoints_ = 5; npoints = npoints_;
				double tmp[npoints_][3] = { {5,-1,-1}, {5,-1,1}, {5,1,1}, {5,1,-1}, {5,0,0} };
				memcpy(points, tmp, npoints*3*sizeof(double));
				break;
			}
			case 3: {
				// square
				const int npoints_ = 5; npoints = npoints_;
				double tmp[npoints_][3] = { {5,-1,-1}, {5,-1,1}, {5,1,1}, {5,1,-1}, {4,0,0} };
				memcpy(points, tmp, npoints*3*sizeof(double));
				break;
			}
			default: npoints = 0;
		}
		
		// add landmarks
		for(int i = 0; i < npoints; ++i)
		{
			pose(0) = points[i][0]; pose(1) = points[i][1]; pose(2) = points[i][2];
			simu::Landmark *lmk = new simu::Landmark(LandmarkAbstract::POINT, pose);
			simulator->addLandmark(lmk);
		}
	}


	// 2. Create robots.
	robot_ptr_t robPtr1;
	if (intOpts[iRobot] == 0) // constant velocity
	{
		robconstvel_ptr_t robPtr1_(new RobotConstantVelocity(mapPtr));
		robPtr1_->setVelocityStd(UNCERT_VLIN, UNCERT_VANG);

		double _v[6] = {
				PERT_VLIN, PERT_VLIN, PERT_VLIN,
				PERT_VANG, PERT_VANG, PERT_VANG };
		vec pertStd = createVector<6>(_v);
		robPtr1_->perturbation.set_std_continuous(pertStd);
		robPtr1_->constantPerturbation = false;

		robPtr1 = robPtr1_;
		
		if (intOpts[iTrigger] == 1)
		{
			// just to initialize the MTI as an external trigger controlling shutter time
			hardware::HardwareEstimatorMti hardEst1(
				MTI_DEVICE, floatOpts[fFreq], floatOpts[fShutter], 1, mode, strOpts[sDataPath], false);
			floatOpts[fFreq] = hardEst1.getFreq();
		}
	}
	else
	if (intOpts[iRobot] == 1) // inertial
	{
		robinertial_ptr_t robPtr1_(new RobotInertial(mapPtr));
		robPtr1_->setInitialStd(UNCERT_VLIN, UNCERT_ABIAS, UNCERT_WBIAS, UNCERT_GRAVITY);

		double _v[12] = {
				PERT_AERR, PERT_AERR, PERT_AERR,
				PERT_WERR, PERT_WERR, PERT_WERR,
				PERT_RANWALKACC, PERT_RANWALKACC, PERT_RANWALKACC,
				PERT_RANWALKGYRO, PERT_RANWALKGYRO, PERT_RANWALKGYRO};
		vec pertStd = createVector<12>(_v);
		robPtr1_->perturbation.set_std_continuous(pertStd);
		robPtr1_->constantPerturbation = false;

		hardware::hardware_estimator_ptr_t hardEst1;
		if (intOpts[iSimu] != 0)
		{
			// TODO
			//boost::shared_ptr<hardware::HardwareEstimatorSimu> hardEst1_(new hardware::HardwareEstimatorSimu(
			//	"/dev/ttyS0", floatOpts[fFreq], floatOpts[fShutter], 1024, mode, strOpts[sDataPath]));
			//hardEst1 = hardEst1_;
		} else
		{
			boost::shared_ptr<hardware::HardwareEstimatorMti> hardEst1_(new hardware::HardwareEstimatorMti(
				MTI_DEVICE, floatOpts[fFreq], floatOpts[fShutter], 1024, mode, strOpts[sDataPath], true));
			if (intOpts[iTrigger]) floatOpts[fFreq] = hardEst1_->getFreq();
			hardEst1_->setSyncConfig(0.009);
			hardEst1 = hardEst1_;
		}
		robPtr1_->setHardwareEstimator(hardEst1);

		robPtr1 = robPtr1_;
	}

	robPtr1->setId();
	robPtr1->linkToParentMap(mapPtr);
	robPtr1->pose.x(quaternion::originFrame());
	if (dataLogger) dataLogger->addLoggable(*robPtr1.get());

	if (intOpts[iSimu] != 0)
	{
		simu::Robot *rob = new simu::Robot(robPtr1->id(), 6);
		if (dataLogger) dataLogger->addLoggable(*rob);
		
		switch (intOpts[iSimu]%10)
		{
			// horiz loop, no rotation
			case 1: {
				double VEL = 0.5;
				rob->addWaypoint(0,0,0, 0,0,0, 0,0,0, 0,0,0);
				rob->addWaypoint(1,0,0, 0,0,0, VEL,0,0, 0,0,0);
				rob->addWaypoint(3,2,0, 0,0,0, 0,VEL,0, 0,0,0);
				rob->addWaypoint(1,4,0, 0,0,0, -VEL,0,0, 0,0,0);
				rob->addWaypoint(-1,4,0, 0,0,0, -VEL,0,0, 0,0,0);
				rob->addWaypoint(-3,2,0, 0,0,0, 0,-VEL,0, 0,0,0);
				rob->addWaypoint(-1,0,0, 0,0,0, VEL,0,0, 0,0,0);
				rob->addWaypoint(0,0,0, 0,0,0, 0,0,0, 0,0,0);
				break;
			}
		
			// two coplanar circles
			case 2: {
				double VEL = 0.5;
				rob->addWaypoint(0 ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				rob->addWaypoint(0 ,0 ,+1, 0,0,0, 0,-VEL,0   , 0,0,0);
				rob->addWaypoint(0 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(0 ,0 ,-1, 0,0,0, 0,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0 ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				rob->addWaypoint(0 ,0 ,+1, 0,0,0, 0,-VEL,0   , 0,0,0);
				rob->addWaypoint(0 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(0 ,0 ,-1, 0,0,0, 0,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0 ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				break;
			}
		
			// two non-coplanar circles at constant velocity
			case 3: {
				double VEL = 0.5;
				rob->addWaypoint(0   ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				rob->addWaypoint(0.25,0 ,+1, 0,0,0, VEL/4,-VEL,0   , 0,0,0);
				rob->addWaypoint(0.5 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(0.25,0 ,-1, 0,0,0, -VEL/4,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0    ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				rob->addWaypoint(-0.25,0 ,+1, 0,0,0, -VEL/4,-VEL,0   , 0,0,0);
				rob->addWaypoint(-0.5 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(-0.25,0 ,-1, 0,0,0, VEL/4,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0 ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				break;
			}

			// two non-coplanar circles with start and stop from/to null speed
			case 4: {
				double VEL = 0.5;
				rob->addWaypoint(0   ,+1,0 , 0,0,0, 0,0   ,0, 0,0,0);
				rob->addWaypoint(0   ,+1,0.1 , 0,0,0, 0,0   ,+VEL/2, 0,0,0);
				rob->addWaypoint(0   ,+1,0.5 , 0,0,0, 0,0   ,+VEL/2, 0,0,0);
				rob->addWaypoint(0.25,0 ,+1, 0,0,0, VEL/4,-VEL,0   , 0,0,0);
				rob->addWaypoint(0.5 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(0.25,0 ,-1, 0,0,0, -VEL/4,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0    ,+1,0 , 0,0,0, 0,0   ,+VEL, 0,0,0);
				rob->addWaypoint(-0.25,0 ,+1, 0,0,0, -VEL/4,-VEL,0   , 0,0,0);
				rob->addWaypoint(-0.5 ,-1,0 , 0,0,0, 0,0   ,-VEL, 0,0,0);
				rob->addWaypoint(-0.25,0 ,-1, 0,0,0, VEL/4,+VEL,0   , 0,0,0);
				
				rob->addWaypoint(0 ,+1,-0.5 , 0,0,0, 0,0   ,+VEL/2, 0,0,0);
				rob->addWaypoint(0 ,+1,-0.1 , 0,0,0, 0,0   ,+VEL/2, 0,0,0);
				rob->addWaypoint(0 ,+1,0 , 0,0,0, 0,0   ,0, 0,0,0);
				break;
			}
		}

		simulator->addRobot(rob);
		
	}
	
	
	// 3. Create sensors.
	pinhole_ptr_t senPtr11(new SensorPinHole(robPtr1, MapObject::UNFILTERED));
	senPtr11->setId();
	senPtr11->linkToParentRobot(robPtr1);
	if (intOpts[iRobot] == 1)
	{
		senPtr11->setPose(0,0,0,90,0,90); // x,y,z,roll,pitch,yaw
	} else
	{
		senPtr11->setPose(0,0,0,-90,0,-90);
	}
	//senPtr11->pose.x(quaternion::originFrame());
	senPtr11->params.setImgSize(img_width, img_height);
	senPtr11->params.setIntrinsicCalibration(intrinsic, distortion, CORRECTION_SIZE);
	//JFR_DEBUG("Correction params: " << senPtr11->params.correction);
	senPtr11->params.setMiscellaneous(PIX_NOISE, D_MIN);

	if (intOpts[iSimu] != 0)
	{
		jblas::vec6 pose;
		subrange(pose, 0, 3) = subrange(senPtr11->pose.x(), 0, 3);
		subrange(pose, 3, 6) = quaternion::q2e(subrange(senPtr11->pose.x(), 3, 7));
		std::swap(pose(3), pose(5)); // FIXME-EULER-CONVENTION
		simu::Sensor *sen = new simu::Sensor(senPtr11->id(), pose, senPtr11);
		simulator->addSensor(robPtr1->id(), sen);
		simulator->addObservationModel(robPtr1->id(), senPtr11->id(), LandmarkAbstract::POINT, new ObservationModelPinHoleEuclideanPoint(senPtr11));
	}
	
	// 3b. Create data manager.
	boost::shared_ptr<ActiveSearchGrid> asGrid(new ActiveSearchGrid(img_width, img_height, GRID_HCELLS, GRID_VCELLS, GRID_MARGIN, GRID_SEPAR));
	
	if (intOpts[iSimu] != 0)
	{
		boost::shared_ptr<simu::DetectorSimu<image::ConvexRoi> > detector(new simu::DetectorSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, PATCH_SIZE, PIX_NOISE, PIX_NOISE*PIX_NOISE_SIMUFACTOR));
		boost::shared_ptr<simu::MatcherSimu<image::ConvexRoi> > matcher(new simu::MatcherSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, PATCH_SIZE, MAX_SEARCH_SIZE, RANSAC_LOW_INNOV, MATCH_TH, MAHALANOBIS_TH, PIX_NOISE, PIX_NOISE*PIX_NOISE_SIMUFACTOR));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac_Simu> dmPt11(new DataManager_ImagePoint_Ransac_Simu(detector, matcher, asGrid, N_UPDATES_TOTAL, N_UPDATES_RANSAC, RANSAC_NTRIES, N_INIT, N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);
		
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorAdhocSimulator(rawdata_condition, rawdata_mutex, floatOpts[fFreq], simulator, senPtr11->id(), robPtr1->id()));
		senPtr11->setHardwareSensor(hardSen11);
	} else
	{
		//boost::shared_ptr<DescriptorImagePointFirstViewFactory> descFactory(new DescriptorImagePointFirstViewFactory(DESC_SIZE));
		boost::shared_ptr<DescriptorImagePointMultiViewFactory> descFactory(new DescriptorImagePointMultiViewFactory(DESC_SIZE, DESC_SCALE_STEP, DESC_ANGLE_STEP, DESC_PREDICTION_TYPE));
		boost::shared_ptr<ImagePointHarrisDetector> harrisDetector(new ImagePointHarrisDetector(HARRIS_CONV_SIZE, HARRIS_TH, HARRIS_EDDGE, PATCH_SIZE, PIX_NOISE, descFactory));
		boost::shared_ptr<ImagePointZnccMatcher> znccMatcher(new ImagePointZnccMatcher(MIN_SCORE, PARTIAL_POSITION, PATCH_SIZE, MAX_SEARCH_SIZE, RANSAC_LOW_INNOV, MATCH_TH, MAHALANOBIS_TH, PIX_NOISE));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac> dmPt11(new DataManager_ImagePoint_Ransac(harrisDetector, znccMatcher, asGrid, N_UPDATES_TOTAL, N_UPDATES_RANSAC, RANSAC_NTRIES, N_INIT, N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);

		
		#ifdef HAVE_VIAM
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, rawdata_mutex, 
			CAMERA_DEVICE, cv::Size(img_width,img_height), 0, 8, floatOpts[fFreq], intOpts[iTrigger], mode, strOpts[sDataPath]));
		senPtr11->setHardwareSensor(hardSen11);
		#else
		if (intOpts[iReplay])
		{
			hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, rawdata_mutex, cv::Size(img_width,img_height),strOpts[sDataPath]));
			senPtr11->setHardwareSensor(hardSen11);
		}
		#endif
	}
	
	//--- force a first display with empty slam to ensure that all windows are loaded
// cout << "SLAM: forcing first initialization display" << endl;
	#ifdef HAVE_MODULE_QDISPLAY
	display::ViewerQt *viewerQt = NULL;
	if (intOpts[iDispQt])
	{
		viewerQt = PTR_CAST<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
		viewerQt->bufferize(*world);
		
		// initializing stuff for controlling run/pause from viewer
		boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
		viewerQt->runStatus.pause = intOpts[iPause];
		viewerQt->runStatus.render_all = intOpts[iRenderAll];
		runStatus_lock.unlock();
	}
	#endif
	#ifdef HAVE_MODULE_GDHE
	display::ViewerGdhe *viewerGdhe = NULL;
	if (intOpts[iDispGdhe])
	{
		viewerGdhe = PTR_CAST<display::ViewerGdhe*> ((*world)->getDisplayViewer(display::ViewerGdhe::id()));
		viewerGdhe->bufferize(*world);
	}
	#endif

	{
		boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
		(*world)->display_rendered = false;
		display_lock.unlock();
		(*world)->display_condition.notify_all();
// cout << "SLAM: now waiting for this display to finish" << endl;
		display_lock.lock();
		while(!(*world)->display_rendered) (*world)->display_condition.wait(display_lock);
		display_lock.unlock();
	}

// cout << "SLAM: starting slam" << endl;

	// Show empty map
	cout << *mapPtr << endl;

	//worldPtr->display_mutex.unlock();

jblas::vec robot_prediction;
double average_robot_innovation = 0.;
int n_innovation = 0;
	
	// ---------------------------------------------------------------------------
	// --- LOOP ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : complete observations set
	// loop all sensors
	// loop all lmks
	// create sen--lmk observation
	// Temporal loop
	if (dataLogger) dataLogger->log();
	kernel::Chrono chrono;
	double max_dt = 0;
	for (; (*world)->t <= N_FRAMES;)
	{
		if ((*world)->exit()) break;
		
#if EVENT_BASED_RAW
		boost::unique_lock<boost::mutex> rawdata_lock(rawdata_mutex);
#endif
		bool had_data = false;
		bool no_more_data = true;
		//worldPtr->display_mutex.lock();
		//if (intOpts[iRenderAll] && worldPtr->display_rendered != (*world)->t)
		//	{ worldPtr->display_mutex.unlock(); boost::this_thread::yield(); continue; }
		chrono.reset();

		// FIXME if intOpts[iRenderAll], manage multisensors : ensure that only 1 sensor is processed, save its time to log it, and ensure that next time next sensor will be tried first
		// FIRST LOOP FOR MEASUREMENT SPACES - ALL DM
		// foreach robot
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
			robIter != mapPtr->robotList().end(); ++robIter)
		{
			robot_ptr_t robPtr = *robIter;
			// cout << "\n================================================== " << endl;
			// cout << *robPtr << endl;

			// foreach sensor
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin();
				senIter != robPtr->sensorList().end(); ++senIter)
			{
				sensor_ptr_t senPtr = *senIter;
				//					cout << "\n________________________________________________ " << endl;
				//					cout << *senPtr << endl;

				// get raw-data
				int r = senPtr->acquireRaw();
				if (r != -2) no_more_data = false;
				if (r < 0)
				{
#if !EVENT_BASED_RAW
					boost::this_thread::yield();
#endif
					continue;
				} else
				{
					had_data=true;
#if EVENT_BASED_RAW
					rawdata_lock.unlock();
#endif
				}
// cout << "\n************************************************** " << endl;
JFR_DEBUG("                 FRAME : " << (*world)->t);
//				cout << "Robot: " << *robPtr << endl;
//				cout << "Pert: " << robPtr->perturbation.P() << "\nPert Jac: " << robPtr->XNEW_pert << "\nState pert: " << robPtr->Q << endl;
//				cout << "Robot state: " << robPtr->state.x() << endl;

				// move the filter time to the data raw.
				robPtr->move(senPtr->getRaw()->timestamp);
JFR_DEBUG("Robot state after move " << robPtr1->state.x());
JFR_DEBUG("Robot state stdev after move " << sqrt(ublas::matrix_vector_range<jblas::sym_mat_indirect>(robPtr1->state.P(), ublas::range(0, robPtr1->state.P().size1()), ublas::range (0, robPtr1->state.P().size2()))));


robot_prediction = robPtr->state.x();

				// foreach dataManager
				for (SensorAbstract::DataManagerList::iterator dmaIter = senPtr->dataManagerList().begin();
					dmaIter != senPtr->dataManagerList().end(); ++dmaIter)
				{
					data_manager_ptr_t dmaPtr = *dmaIter;
					dmaPtr->process(senPtr->getRaw());
				} // foreach dataManager

average_robot_innovation += ublas::norm_2(robPtr->state.x() - robot_prediction);
n_innovation++;

			} // for each sensor
		} // for each robot

		// NOW LOOP FOR STATE SPACE - ALL MM
		if (had_data)
		{
JFR_DEBUG("Robot state after corrections " << robPtr1->state.x());
JFR_DEBUG("Robot state stdev after corrections " << sqrt(ublas::matrix_vector_range<jblas::sym_mat_indirect>(robPtr1->state.P(), ublas::range(0, robPtr1->state.P().size1()), ublas::range (0, robPtr1->state.P().size2()))));
			if (robPtr1->dt_or_dx > max_dt) max_dt = robPtr1->dt_or_dx;

			// Output info
//						cout << endl;
//						cout << "dt: " << (int) (1000 * robPtr1->dt_or_dx) << "ms (match "
//						<< total_match_time/1000 << " ms, update " << total_update_time/1000 << "ms). Lmk: [";
//						cout << mmPoint->landmarkList().size() << "] ";
//						for (MapManagerAbstract::LandmarkList::iterator lmkIter =
//								mmPoint->landmarkList().begin(); lmkIter
//								!= mmPoint->landmarkList().end(); lmkIter++) {
//							cout << (*lmkIter)->id() << " ";
//						}

			for (MapAbstract::MapManagerList::iterator mmIter = mapPtr->mapManagerList().begin(); 
				mmIter != mapPtr->mapManagerList().end(); ++mmIter)
			{
				map_manager_ptr_t mapMgr = *mmIter;
				mapMgr->manage();
			}
// cout << "SLAM: processed a frame: t " << (*world)->t << " display_t " << (*world)->display_t << endl;

			bool renderAll;
			#ifdef HAVE_MODULE_QDISPLAY
			if (intOpts[iDispQt])
			{
				boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
				renderAll = viewerQt->runStatus.render_all;
				runStatus_lock.unlock();
			} else
			#endif
			renderAll = (intOpts[iRenderAll] != 0);

			if (renderAll)
			{
				boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
				while(!(*world)->display_rendered && !(*world)->exit()) (*world)->display_condition.wait(display_lock);
				display_lock.unlock();
			}

		} // if had_data

/*		if (no_more_data) // we wait the display to ensure that the last frame is displayed
		{
			boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
			while(!(*world)->display_rendered) (*world)->display_condition.wait(display_lock);
			display_lock.unlock();
		}
*/
		unsigned processed_t = (had_data ? (*world)->t : (*world)->t-1);
		if ((*world)->display_t+1 < processed_t+1)
		{
//cout << "SLAM: checking if display has rendered" << endl;
			boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
			if ((*world)->display_rendered)
			{
// cout << "SLAM: display has finished, let's bufferize this one" << endl;
				#ifdef HAVE_MODULE_QDISPLAY
				display::ViewerQt *viewerQt = NULL;
				if (intOpts[iDispQt]) viewerQt = PTR_CAST<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
				if (intOpts[iDispQt]) viewerQt->bufferize(*world);
				#endif
				#ifdef HAVE_MODULE_GDHE
				display::ViewerGdhe *viewerGdhe = NULL;
				if (intOpts[iDispGdhe]) viewerGdhe = PTR_CAST<display::ViewerGdhe*> ((*world)->getDisplayViewer(display::ViewerGdhe::id()));
				if (intOpts[iDispGdhe]) viewerGdhe->bufferize(*world);
				#endif
				
				(*world)->display_t = (*world)->t;
				(*world)->display_rendered = false;
				display_lock.unlock();
				(*world)->display_condition.notify_all();
			} else
			display_lock.unlock();
		}
		
		if (no_more_data) break;

#if EVENT_BASED_RAW
		if (!had_data)
		{
			rawdata_condition.wait(rawdata_lock);
			rawdata_lock.unlock();
		}
#endif
		
		
//		int t = (*world)->t;
//		worldPtr->display_mutex.unlock();

		bool doPause;
		#ifdef HAVE_MODULE_QDISPLAY
		if (intOpts[iDispQt])
		{
			boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
			doPause = viewerQt->runStatus.pause;
			runStatus_lock.unlock();
		} else
		#endif
		doPause = (intOpts[iPause] != 0);
		if (doPause && had_data && !(*world)->exit())
		{
			(*world)->slam_blocked(true);
			#ifdef HAVE_MODULE_QDISPLAY
			if (intOpts[iDispQt])
			{
				boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
				do {
					viewerQt->runStatus.condition.wait(runStatus_lock);
				} while (viewerQt->runStatus.pause && !viewerQt->runStatus.next);
				viewerQt->runStatus.next = 0;
				runStatus_lock.unlock();
			} else
			#endif
			getchar(); // wait for key in replay mode
			(*world)->slam_blocked(false);
		}
//std::cout << "one frame " << (*world)->t << " : " << mode << " " << had_data << std::endl;

		if (had_data)
		{
			(*world)->t++;
			if (dataLogger) dataLogger->log();
		}
	} // temporal loop

average_robot_innovation /= n_innovation;
std::cout << "average_robot_innovation " << average_robot_innovation << std::endl;

	(*world)->slam_blocked(true);
//	std::cout << "\nFINISHED ! Press a key to terminate." << std::endl;
//	getchar();
} // demo_slam01_main


void demo_slam01_display(world_ptr_t *world) {
//	static unsigned prev_t = 0;
	kernel::Timer timer(display_period*1000);
	while(true)
	{
		/*
		if (intOpts[iDispQt])
		{
			#ifdef HAVE_MODULE_QDISPLAY
			qdisplay::qtMutexLock<kernel::FifoMutex>((*world)->display_mutex);
			#endif
		}
		else
		{
			(*world)->display_mutex.lock();
		}
		*/
		// just to display the last frame if slam is blocked or has finished
		boost::unique_lock<kernel::VariableMutex<bool> > blocked_lock((*world)->slam_blocked);
		if ((*world)->slam_blocked.var)
		{
			if ((*world)->display_t+1 < (*world)->t+1 && (*world)->display_rendered)
			{
				#ifdef HAVE_MODULE_QDISPLAY
				display::ViewerQt *viewerQt = NULL;
				if (intOpts[iDispQt]) viewerQt = PTR_CAST<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
				if (intOpts[iDispQt]) viewerQt->bufferize(*world);
				#endif
				#ifdef HAVE_MODULE_GDHE
				display::ViewerGdhe *viewerGdhe = NULL;
				if (intOpts[iDispGdhe]) viewerGdhe = PTR_CAST<display::ViewerGdhe*> ((*world)->getDisplayViewer(display::ViewerGdhe::id()));
				if (intOpts[iDispGdhe]) viewerGdhe->bufferize(*world);
				#endif
				
				(*world)->display_t = (*world)->t;
				(*world)->display_rendered = false;
			}
		}
		blocked_lock.unlock();
		
		// waiting that display is ready
// cout << "DISPLAY: waiting for data" << endl;
		boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
		if (intOpts[iDispQt] == 0)
		{
			while((*world)->display_rendered)
				(*world)->display_condition.wait(display_lock);
		} else
		{
			#ifdef HAVE_MODULE_QDISPLAY
			int nwait = std::max(1,display_period/10-1);
			for(int i = 0; (*world)->display_rendered && i < nwait; ++i)
			{
				(*world)->display_condition.timed_wait(display_lock, boost::posix_time::milliseconds(10));
				display_lock.unlock();
				QApplication::instance()->processEvents();
				display_lock.lock();
			}
			if ((*world)->display_rendered) break;
			#endif
		}
		display_lock.unlock();
// cout << "DISPLAY: ok data here, let's start!" << endl;

//		if ((*world)->t != prev_t)
//		{
//			prev_t = (*world)->t;
//			(*world)->display_rendered = (*world)->t;
			
//		!bufferize!

//			if (!intOpts[iRenderAll]) // strange: if we always unlock here, qt.dump takes much more time...
//				(*world)->display_mutex.unlock();
				
			#ifdef HAVE_MODULE_QDISPLAY
			display::ViewerQt *viewerQt = NULL;
			if (intOpts[iDispQt]) viewerQt = PTR_CAST<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
			if (intOpts[iDispQt]) viewerQt->render();
			#endif
			#ifdef HAVE_MODULE_GDHE
			display::ViewerGdhe *viewerGdhe = NULL;
			if (intOpts[iDispGdhe]) viewerGdhe = PTR_CAST<display::ViewerGdhe*> ((*world)->getDisplayViewer(display::ViewerGdhe::id()));
			if (intOpts[iDispGdhe]) viewerGdhe->render();
			#endif
			
			if ((intOpts[iReplay] || intOpts[iSimu]) && intOpts[iDump] && (*world)->display_t+1 != 0)
			{
				#ifdef HAVE_MODULE_QDISPLAY
				if (intOpts[iDispQt])
				{
					std::ostringstream oss; oss << strOpts[sDataPath] << "/rendered-2D_%d-" << std::setw(6) << std::setfill('0') << (*world)->display_t << ".png";
					viewerQt->dump(oss.str());
				}
				#endif
				#ifdef HAVE_MODULE_GDHE
				if (intOpts[iDispGdhe])
				{
					std::ostringstream oss; oss << strOpts[sDataPath] << "/rendered-3D_" << std::setw(6) << std::setfill('0') << (*world)->display_t << ".png";
					viewerGdhe->dump(oss.str());
				}
				#endif
//				if (intOpts[iRenderAll])
//					(*world)->display_mutex.unlock();
			}
//		} else
//		{
//			(*world)->display_mutex.unlock();
//			boost::this_thread::yield();
//		}
// cout << "DISPLAY: finished display, marking rendered" << endl;
		display_lock.lock();
		(*world)->display_rendered = true;
		display_lock.unlock();
		(*world)->display_condition.notify_all();
		
		if (intOpts[iDispQt]) break; else timer.wait();
	}
}

void demo_slam01_exit(world_ptr_t *world, boost::thread *thread_main) {
	(*world)->exit(true);
	(*world)->display_condition.notify_all();
// 	std::cout << "EXITING !!!" << std::endl;
	//fputc('\n', stdin);
	thread_main->timed_join(boost::posix_time::milliseconds(500));
}


	void demo_slam01() {
		world_ptr_t worldPtr(new WorldAbstract());

		// deal with the random seed
		rseed = time(NULL);
		if (intOpts[iRandSeed] != 0 && intOpts[iRandSeed] != 1)
			rseed = intOpts[iRandSeed];
		if (!intOpts[iReplay] && intOpts[iDump]) {
			std::fstream f((strOpts[sDataPath] + std::string("/rseed.log")).c_str(), std::ios_base::out);
			f << rseed << std::endl;
			f.close();
		}
		else if (intOpts[iReplay] && intOpts[iRandSeed] == 1) {
			std::fstream f((strOpts[sDataPath] + std::string("/rseed.log")).c_str(), std::ios_base::in);
			f >> rseed;
			f.close();
		}
		std::cout << __FILE__ << ":" << __LINE__ << " rseed " << rseed << std::endl;
		rtslam::srand(rseed);

		#ifdef HAVE_MODULE_QDISPLAY
		if (intOpts[iDispQt])
		{
			display::ViewerQt *viewerQt = new display::ViewerQt(8, MAHALANOBIS_TH, false, "data/rendered2D_%02d-%06d.png");
			worldPtr->addDisplayViewer(viewerQt, display::ViewerQt::id());
		}
		#endif
		#ifdef HAVE_MODULE_GDHE
		if (intOpts[iDispGdhe])
		{
			display::ViewerGdhe *viewerGdhe = new display::ViewerGdhe("camera", MAHALANOBIS_TH, "localhost");
			boost::filesystem::path ram_path("/mnt/ram");
			if (boost::filesystem::exists(ram_path) && boost::filesystem::is_directory(ram_path))
				viewerGdhe->setConvertTempPath("/mnt/ram");
			worldPtr->addDisplayViewer(viewerGdhe, display::ViewerGdhe::id());
		}
		#endif

		// to start with qt display
		if (intOpts[iDispQt]) // at least 2d
		{
			#ifdef HAVE_MODULE_QDISPLAY
			qdisplay::QtAppStart((qdisplay::FUNC)&demo_slam01_display,display_priority,(qdisplay::FUNC)&demo_slam01_main,slam_priority,display_period,&worldPtr,(qdisplay::EXIT)&demo_slam01_exit);
			#else
			std::cout << "Please install qdisplay module if you want 2D display" << std::endl;
			#endif
		} else
		if (intOpts[iDispGdhe]) // only 3d
		{
			#ifdef HAVE_MODULE_GDHE
			kernel::setCurrentThreadPriority(display_priority);
			boost::thread *thread_disp = new boost::thread(boost::bind(demo_slam01_display,&worldPtr));
			kernel::setCurrentThreadPriority(slam_priority);
			demo_slam01_main(&worldPtr);
			delete thread_disp;
			#else
			std::cout << "Please install gdhe module if you want 3D display" << std::endl;
			#endif
		} else // none
		{
			kernel::setCurrentThreadPriority(slam_priority);
			demo_slam01_main(&worldPtr);
		}

		JFR_DEBUG("Terminated");
	}




		/**
		 * Program options:
		 * --disp-2d=0/1
		 * --disp-3d=0/1
		 * --render-all=0/1 (needs --replay 1)
		 * --replay=0/1 (needs --data-path)
		 * --dump=0/1  (needs --data-path)
		 * --rand-seed=0/1/n, 0=generate new one, 1=in replay use the saved one, n=use seed n
		 * --pause=0/n 0=don't, n=pause for frames>n (needs --replay 1)
		 * --log=0/1 -> log result in text file
		 * --verbose=0/1/2/3/4/5 -> Off/Trace/Warning/Debug/VerboseDebug/VeryVerboseDebug
		 * --data-path=/mnt/ram/rtslam
		 * #--slam-config=data/config1.xml -> not implemented yet
		 * --help
		 * --usage
		 * --robot 0=constant vel, 1=inertial
		 * --trigger 0=internal, 1=external with MTI control, 2=external without control
		 * --simu 0/1
		 * --freq camera frequency in double Hz (with trigger==0/1)
		 * --shutter shutter time in double seconds (with trigger==1)
		 *
		 * You can use the following examples and only change values:
		 * online test (old mode=0):
		 *   demo_slam --disp-2d=1 --disp-3d=1 --render-all=0 --replay=0 --dump=0 --rand-seed=0 --pause=0 --data-path=data/rtslam01
		 *   demo_slam --disp-2d=1 --disp-3d=1
		 * online with dump (old mode=1):
		 *   demo_slam --disp-2d=1 --disp-3d=1 --render-all=0 --replay=0 --dump=1 --rand-seed=0 --pause=0 --data-path=data/rtslam01
		 *   demo_slam --disp-2d=1 --disp-3d=1 --dump=1 --data-path=data/rtslam01
		 * replay with pause  (old mode=2):
		 *   demo_slam --disp-2d=1 --disp-3d=1 --render-all=1 --replay=1 --dump=0 --rand-seed=1 --pause=1 --data-path=data/rtslam01
		 * replay with dump  (old mode=3):
		 *   demo_slam --disp-2d=1 --disp-3d=1 --render-all=1 --replay=1 --dump=1 --rand-seed=1 --pause=0 --data-path=data/rtslam01
		 */
		int main(int argc, char* const* argv)
		{
			intOpts[iVerbose] = 5;
			floatOpts[fFreq] = 60.0;
			floatOpts[fShutter] = 2e-3;
			strOpts[sDataPath] = ".";
			
			while (1)
			{
				int c, option_index = 0;
				c = getopt_long_only(argc, argv, "", long_options, &option_index);
				if (c == -1) break;
				if (c == 0)
				{
					if (option_index <= nLastIntOpt)
					{
						intOpts[option_index] = 1;
						if (optarg) intOpts[option_index-nFirstIntOpt] = atoi(optarg);
					} else
					if (option_index <= nLastFloatOpt)
					{
						if (optarg) floatOpts[option_index-nFirstFloatOpt] = atof(optarg);
					} else
					if (option_index <= nLastStrOpt)
					{
						if (optarg) strOpts[option_index-nFirstStrOpt] = optarg;
					} else
					{
						std::cout << "Integer options:" << std::endl;
						for(int i = 0; i < nIntOpts; ++i)
							std::cout << "\t--" << long_options[i+nFirstIntOpt].name << std::endl;
						
						std::cout << "Float options:" << std::endl;
						for(int i = 0; i < nFloatOpts; ++i)
							std::cout << "\t--" << long_options[i+nFirstFloatOpt].name << std::endl;

						std::cout << "String options:" << std::endl;
						for(int i = 0; i < nStrOpts; ++i)
							std::cout << "\t--" << long_options[i+nFirstStrOpt].name << std::endl;
						
						std::cout << "Breaking options:" << std::endl;
						for(int i = 0; i < 2; ++i)
							std::cout << "\t--" << long_options[i+nFirstBreakingOpt].name << std::endl;
						
						return 0;
					}
				} else
				{
					std::cerr << "Unknown option " << c << std::endl;
				}
			}

			// consistency
			if (intOpts[iReplay]) mode = 2; else
				if (intOpts[iDump]) mode = 1; else
					mode = 0;
			#ifndef HAVE_MODULE_QDISPLAY
			intOpts[iDispQt] = 0;
			#endif
			#ifndef HAVE_MODULE_GDHE
			intOpts[iDispGdhe] = 0;
			#endif

			demo_slam01();
		}
