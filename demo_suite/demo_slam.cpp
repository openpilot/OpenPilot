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


/** ############################################################################
 * #############################################################################
 * features enable/disable
 * ###########################################################################*/

/*
 * STATUS: working fine, use it
 * Ransac ensures that we use correct observations for a few first updates,
 * allowing bad observations to be more easily detected by gating
 * You can disable it by setting N_UPDATES_RANSAC to 0
 */
#define RANSAC_FIRST 1

/*
 * STATUS: working fine, use it
 * This allows to have 0% cpu used for waiting/idle
 */
#define EVENT_BASED_RAW 1

/*
 * STATUS: in progress, do not use for now
 * This allows to track landmarks longer by updating the reference patch when
 * the landmark is not detected anymore and the point of view has changed
 * significantly enough
 * The problem is that the correlation is not robust enough in a matching
 * (opposed to tracking) context, and it can provoke matching errors with a
 * progressive appearance drift.
 * Also decreasing perfs by 10%, probably because we save a view at each obs,
 * or maybe it just because of the different random process
 */
#define MULTIVIEW_DESCRIPTOR 0

/*
 * STATUS: in progress, do not use for now
 * This allows to ignore some landmarks when we have some experience telling
 * us that we can't observe this landmarks from here (masking), in order to
 * save some time, and to allow creation of other observations in the
 * neighborhood to keep localizing with enough landmarks.
 * The problem is that sometimes it creates too many landmarks in the same area
 * (significantly slowing down slam), and sometimes doesn't create enough of
 * them when it is necessary.
 */
#define VISIBILITY_MAP 0


/*
 * STATUS: in progress, do not use for now
 * Only update if innovation is significant wrt measurement uncertainty.
 * 
 * Large updates are causing inconsistency because of linearization errors,
 * but too numerous updates are also causing inconsistency, 
 * so we should avoid to do not significant updates. 
 * An update is not significant if there are large odds that it is 
 * only measurement noise and that there is not much information.
 * 
 * When the camera is not moving at all, the landmarks are converging anyway
 * quite fast because of this, at very unconsistent positions of course, 
 * so that when the camera moves it cannot recover them.
 * 
 * Some work needs to be done yet to prevent search ellipses from growing
 * too much and integrate it better with the whole management, but this was
 * for first evaluation purpose.
 * 
 * Unfortunately it doesn't seem to improve much the situation, even if
 * it is still working correctly with less computations.
 * The feature is disabled for now.
 */
#define RELEVANCE_TEST 0


/** ############################################################################
 * #############################################################################
 * Includes
 * ###########################################################################*/

#include <iostream>
#include <boost/shared_ptr.hpp>
//#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <time.h>
#include <map>
#include <getopt.h>
#include "kernel/keyValueFile.hpp"

// jafar debug include
#include "kernel/jafarDebug.hpp"
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
#include "rtslam/hardwareEstimatorInertialAdhocSimulator.hpp"


/** ############################################################################
 * #############################################################################
 * types variables functions
 * ###########################################################################*/

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

int mode = 0;
time_t rseed;


/** ############################################################################
 * #############################################################################
 * program parameters
 * ###########################################################################*/

enum { iDispQt = 0, iDispGdhe, iRenderAll, iReplay, iDump, iRandSeed, iPause, iLog, iVerbose, iMap, iRobot, iTrigger, iSimu, nIntOpts };
int intOpts[nIntOpts] = {0};
const int nFirstIntOpt = 0, nLastIntOpt = nIntOpts-1;

enum { fFreq = 0, fShutter, nFloatOpts };
double floatOpts[nFloatOpts] = {0.0};
const int nFirstFloatOpt = nIntOpts, nLastFloatOpt = nIntOpts+nFloatOpts-1;

enum { sDataPath = 0, sConfigSetup, sConfigEstimation, nStrOpts };
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
	{"map", 2, 0, 0},
	{"robot", 2, 0, 0}, // should be in config file
	{"trigger", 2, 0, 0}, // should be in config file
	{"simu", 2, 0, 0},
	// double options
	{"freq", 2, 0, 0}, // should be in config file
	{"shutter", 2, 0, 0}, // should be in config file
	// string options
	{"data-path", 1, 0, 0},
	{"config-setup", 1, 0, 0},
	{"config-estimation", 1, 0, 0},
	// breaking options
	{"help",0,0,0},
	{"usage",0,0,0},
};


/** ############################################################################
 * #############################################################################
 * Config data
 * ###########################################################################*/

const int slam_priority = -20; // needs to be started as root to be < 0
const int display_priority = 10;
const int display_period = 100; // ms
const unsigned N_FRAMES = 500000;


class ConfigSetup: public kernel::KeyValueFileSaveLoad
{
 public:
	/// SENSOR
	jblas::vec6 SENSOR_POSE_CONSTVEL; /// sensor pose in constant velocity (x,y,z,roll,pitch,yaw) (deg)
	jblas::vec6 SENSOR_POSE_INERTIAL; /// sensor pose in inertial (x,y,z,roll,pitch,yaw) (deg)

	std::string CAMERA_DEVICE; /// camera device (firewire ID or device)
	unsigned IMG_WIDTH;        /// image width
	unsigned IMG_HEIGHT;       /// image height
	jblas::vec4 INTRINSIC;     /// intrisic calibration parameters (u0,v0,alphaU,alphaV)
	jblas::vec3 DISTORTION;    /// distortion calibration parameters

	/// SIMU SENSOR
	unsigned IMG_WIDTH_SIMU;
	unsigned IMG_HEIGHT_SIMU;
	jblas::vec4 INTRINSIC_SIMU;
	jblas::vec3 DISTORTION_SIMU;

	/// CONSTANT VELOCITY
	double UNCERT_VLIN; /// initial uncertainty stdev on linear velocity (m/s)
	double UNCERT_VANG; /// initial uncertainty stdev on angular velocity (rad/s)
	double PERT_VLIN;   /// perturbation on linear velocity, ie non-constantness (m/s per sqrt(s))
	double PERT_VANG;   /// perturbation on angular velocity, ie non-constantness (rad/s per sqrt(s))

	/// INERTIAL (also using UNCERT_VLIN)
	std::string MTI_DEVICE;    /// IMU device
	double ACCELERO_FULLSCALE; /// full scale of accelerometers (m/s2)  (MTI: 17)
	double ACCELERO_NOISE;     /// noise stdev of accelerometers (m/s2) (MTI: 0.002*sqrt(30) )
	double GYRO_FULLSCALE;     /// full scale of gyrometers (rad/s)     (MTI: rad(300) )
	double GYRO_NOISE;         /// noise stdev of gyrometers (rad/s)    (MTI: rad(0.05)*sqrt(40) )

	double UNCERT_GRAVITY;   /// initial gravity uncertainty (% of 9.81)
	double UNCERT_ABIAS;     /// initial accelerometer bias uncertainty (% of ACCELERO_FULLSCALE)
	double UNCERT_WBIAS;     /// initial gyrometer bias uncertainty (% of GYRO_FULLSCALE)
	double PERT_AERR;        /// noise stdev of accelerometers (% of ACCELERO_NOISE)
	double PERT_WERR;        /// noise stdev of gyrometers (% of GYRO_NOISE)
	double PERT_RANWALKACC;  /// IMU a_bias random walk (m/s2 per sqrt(s))
	double PERT_RANWALKGYRO; /// IMU w_bias random walk (rad/s per sqrt(s))

	double IMU_TIMESTAMP_CORRECTION; /// correction to add to the IMU timestamp for synchronization (s)

	/// SIMU INERTIAL
	double SIMU_IMU_TIMESTAMP_CORRECTION;
	double SIMU_IMU_FREQ;
	double SIMU_IMU_GRAVITY;
	double SIMU_IMU_GYR_BIAS;
	double SIMU_IMU_GYR_BIAS_NOISESTD;
	double SIMU_IMU_GYR_GAIN;
	double SIMU_IMU_GYR_GAIN_NOISESTD;
	double SIMU_IMU_RANDWALKGYR_FACTOR;
	double SIMU_IMU_ACC_BIAS;
	double SIMU_IMU_ACC_BIAS_NOISESTD;
	double SIMU_IMU_ACC_GAIN;
	double SIMU_IMU_ACC_GAIN_NOISESTD;
	double SIMU_IMU_RANDWALKACC_FACTOR;
	
 public:
	virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile);
	virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile);
} configSetup;



class ConfigEstimation: public kernel::KeyValueFileSaveLoad
{
 public:
	/// MISC
	unsigned CORRECTION_SIZE; /// number of coefficients for the distortion correction polynomial

	/// FILTER
	unsigned MAP_SIZE; /// map size in # of states, robot + landmarks
	double PIX_NOISE;  /// measurement noise of a point
	double PIX_NOISE_SIMUFACTOR;

	/// LANDMARKS
	double D_MIN;      /// inverse depth mean initialization
	double REPARAM_TH; /// reparametrization threshold

	unsigned GRID_HCELLS;
	unsigned GRID_VCELLS;
	unsigned GRID_MARGIN;
	unsigned GRID_SEPAR;

	double RELEVANCE_TH;       /// (# of sigmas)
	double MAHALANOBIS_TH;     /// (# of sigmas)
	unsigned N_UPDATES_TOTAL;  /// max number of landmarks to update every frame
	unsigned N_UPDATES_RANSAC; /// max number of landmarks to update with ransac every frame
	unsigned N_INIT;           /// maximum number of landmarks to try to initialize every frame
	unsigned N_RECOMP_GAINS;   /// how many times information gain is recomputed to resort observations in active search
	double RANSAC_LOW_INNOV;   /// ransac low innovation threshold (pixels)

	unsigned RANSAC_NTRIES;    /// number of base observation used to initialize a ransac set

	/// RAW PROCESSING
	unsigned HARRIS_CONV_SIZE;
	double HARRIS_TH;
	double HARRIS_EDDGE;

	unsigned DESC_SIZE;     /// descriptor patch size (odd value)
	double DESC_SCALE_STEP; /// MultiviewDescriptor: min change of scale (ratio)
	double DESC_ANGLE_STEP; /// MultiviewDescriptor: min change of point of view (deg)
	int DESC_PREDICTION_TYPE; /// type of prediction from descriptor (0 = none, 1 = affine, 2 = homographic)

	unsigned PATCH_SIZE;       /// patch size used for matching
	unsigned MAX_SEARCH_SIZE;  /// if the search area is larger than this # of pixels, we bound it
	unsigned KILL_SEARCH_SIZE; /// if the search area is larger than this # of pixels, we vote for killing the landmark
	double MATCH_TH;           /// ZNCC score threshold
	double MIN_SCORE;          /// min ZNCC score under which we don't finish to compute the value of the score
	double PARTIAL_POSITION;   /// position in the patch where we test if we finish the correlation computation
	
 public:
	virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile);
	virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile);
} configEstimation;



/** ############################################################################
 * #############################################################################
 * Slam function
 * ###########################################################################*/


void demo_slam01_main(world_ptr_t *world) {
	
	vec intrinsic, distortion;
	int img_width, img_height;
	if (intOpts[iSimu] != 0)
	{
		img_width = configSetup.IMG_WIDTH_SIMU;
		img_height = configSetup.IMG_HEIGHT_SIMU;
		intrinsic = configSetup.INTRINSIC_SIMU;
		distortion = configSetup.DISTORTION_SIMU;
		
	} else
	{
		img_width = configSetup.IMG_WIDTH;
		img_height = configSetup.IMG_HEIGHT;
		intrinsic = configSetup.INTRINSIC;
		distortion = configSetup.DISTORTION;
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
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpSimuObservationMaker(
		  configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpSimuObservationMaker(
		  configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
	} else
	{
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker(
		  configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker(
		  configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
	}


	// ---------------------------------------------------------------------------
	// --- INIT ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : 1 map and map-manager, 2 robs, 3 sens and data-manager.
	world_ptr_t worldPtr = *world;
	//worldPtr->display_mutex.lock();


	// 1. Create maps.
	map_ptr_t mapPtr(new MapAbstract(configEstimation.MAP_SIZE));
	mapPtr->linkToParentWorld(worldPtr);
	
	// 1b. Create map manager.
	landmark_factory_ptr_t lmkFactory(new LandmarkFactory<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint>());
	map_manager_ptr_t mmPoint;
	switch(intOpts[iMap])
	{
		case 0: { // odometry
			mmPoint.reset(new MapManagerOdometry(
			  lmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			break;
		}
		case 1: { // global
			mmPoint.reset(new MapManagerGlobal(
			  lmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE, 30, 0.5, 0.5));
			break;
		}
		case 2: { // local/multimap
			mmPoint.reset(new MapManagerLocal(
			  lmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			break;	
		}
	}
	mmPoint->linkToParentMap(mapPtr);

	// simulation environment
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
		robPtr1_->setVelocityStd(configSetup.UNCERT_VLIN, configSetup.UNCERT_VANG);
		robPtr1_->setId();

		double _v[6] = {
				configSetup.PERT_VLIN, configSetup.PERT_VLIN, configSetup.PERT_VLIN,
				configSetup.PERT_VANG, configSetup.PERT_VANG, configSetup.PERT_VANG };
		vec pertStd = createVector<6>(_v);
		robPtr1_->perturbation.set_std_continuous(pertStd);
		robPtr1_->constantPerturbation = false;

		robPtr1 = robPtr1_;
		
		if (intOpts[iTrigger] != 0)
		{
			// just to initialize the MTI as an external trigger controlling shutter time
			hardware::HardwareEstimatorMti hardEst1(
				configSetup.MTI_DEVICE, intOpts[iTrigger], floatOpts[fFreq], floatOpts[fShutter], 1, mode, strOpts[sDataPath]);
			floatOpts[fFreq] = hardEst1.getFreq();
		}
	}
	else
	if (intOpts[iRobot] == 1) // inertial
	{
		robinertial_ptr_t robPtr1_(new RobotInertial(mapPtr));
		robPtr1_->setInitialStd(
			configSetup.UNCERT_VLIN,
			configSetup.UNCERT_ABIAS*configSetup.ACCELERO_FULLSCALE,
			configSetup.UNCERT_WBIAS*configSetup.GYRO_FULLSCALE,
			configSetup.UNCERT_GRAVITY*9.81);
		robPtr1_->setId();

		double aerr = configSetup.PERT_AERR * configSetup.ACCELERO_NOISE;
		double werr = configSetup.PERT_WERR * configSetup.GYRO_NOISE;
		double _v[12] = {
				aerr, aerr, aerr, werr, werr, werr,
				configSetup.PERT_RANWALKACC, configSetup.PERT_RANWALKACC, configSetup.PERT_RANWALKACC,
				configSetup.PERT_RANWALKGYRO, configSetup.PERT_RANWALKGYRO, configSetup.PERT_RANWALKGYRO};
		vec pertStd = createVector<12>(_v);
		robPtr1_->perturbation.set_std_continuous(pertStd);
		robPtr1_->constantPerturbation = false;

		hardware::hardware_estimator_ptr_t hardEst1;
		if (intOpts[iSimu] != 0)
		{
			boost::shared_ptr<hardware::HardwareEstimatorInertialAdhocSimulator> hardEst1_(
				new hardware::HardwareEstimatorInertialAdhocSimulator(configSetup.SIMU_IMU_FREQ, 50, simulator, robPtr1_->id()));
			hardEst1_->setSyncConfig(configSetup.SIMU_IMU_TIMESTAMP_CORRECTION);
			
			hardEst1_->setErrors(configSetup.SIMU_IMU_GRAVITY, 
				configSetup.SIMU_IMU_GYR_BIAS, configSetup.SIMU_IMU_GYR_BIAS_NOISESTD,
				configSetup.SIMU_IMU_GYR_GAIN, configSetup.SIMU_IMU_GYR_GAIN_NOISESTD,
				configSetup.SIMU_IMU_RANDWALKGYR_FACTOR * configSetup.PERT_RANWALKGYRO,
				configSetup.SIMU_IMU_ACC_BIAS, configSetup.SIMU_IMU_ACC_BIAS_NOISESTD,
				configSetup.SIMU_IMU_ACC_GAIN, configSetup.SIMU_IMU_ACC_GAIN_NOISESTD,
				configSetup.SIMU_IMU_RANDWALKACC_FACTOR * configSetup.PERT_RANWALKACC);
			
			hardEst1 = hardEst1_;
		} else
		{
			boost::shared_ptr<hardware::HardwareEstimatorMti> hardEst1_(new hardware::HardwareEstimatorMti(
				configSetup.MTI_DEVICE, intOpts[iTrigger], floatOpts[fFreq], floatOpts[fShutter], 1024, mode, strOpts[sDataPath]));
			if (intOpts[iTrigger] != 0) floatOpts[fFreq] = hardEst1_->getFreq();
			hardEst1_->setSyncConfig(configSetup.IMU_TIMESTAMP_CORRECTION);
			hardEst1_->start();
			hardEst1 = hardEst1_;
		}
		robPtr1_->setHardwareEstimator(hardEst1);

		robPtr1 = robPtr1_;
	}

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
		senPtr11->setPose(configSetup.SENSOR_POSE_INERTIAL[0], configSetup.SENSOR_POSE_INERTIAL[1], configSetup.SENSOR_POSE_INERTIAL[2],
											configSetup.SENSOR_POSE_INERTIAL[3], configSetup.SENSOR_POSE_INERTIAL[4], configSetup.SENSOR_POSE_INERTIAL[5]); // x,y,z,roll,pitch,yaw
	} else
	{
		senPtr11->setPose(configSetup.SENSOR_POSE_CONSTVEL[0], configSetup.SENSOR_POSE_CONSTVEL[1], configSetup.SENSOR_POSE_CONSTVEL[2],
											configSetup.SENSOR_POSE_CONSTVEL[3], configSetup.SENSOR_POSE_CONSTVEL[4], configSetup.SENSOR_POSE_CONSTVEL[5]); // x,y,z,roll,pitch,yaw
	}
	//senPtr11->pose.x(quaternion::originFrame());
	senPtr11->params.setImgSize(img_width, img_height);
	senPtr11->params.setIntrinsicCalibration(intrinsic, distortion, configEstimation.CORRECTION_SIZE);
	//JFR_DEBUG("Correction params: " << senPtr11->params.correction);
	senPtr11->params.setMiscellaneous(configEstimation.PIX_NOISE, configEstimation.D_MIN);

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
	boost::shared_ptr<ActiveSearchGrid> asGrid(new ActiveSearchGrid(img_width, img_height, configEstimation.GRID_HCELLS, configEstimation.GRID_VCELLS, configEstimation.GRID_MARGIN, configEstimation.GRID_SEPAR));
	
	#if RANSAC_FIRST
	int ransac_ntries = configEstimation.RANSAC_NTRIES;
	#else
	int ransac_ntries = 0;
	#endif
	if (intOpts[iSimu] != 0)
	{
		boost::shared_ptr<simu::DetectorSimu<image::ConvexRoi> > detector(new simu::DetectorSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, configEstimation.PATCH_SIZE, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));
		boost::shared_ptr<simu::MatcherSimu<image::ConvexRoi> > matcher(new simu::MatcherSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, configEstimation.PATCH_SIZE, configEstimation.MAX_SEARCH_SIZE, configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac_Simu> dmPt11(new DataManager_ImagePoint_Ransac_Simu(detector, matcher, asGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);
		
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorAdhocSimulator(rawdata_condition, rawdata_mutex, floatOpts[fFreq], simulator, senPtr11->id(), robPtr1->id()));
		senPtr11->setHardwareSensor(hardSen11);
	} else
	{
		#if MULTIVIEW_DESCRIPTOR
		boost::shared_ptr<DescriptorImagePointMultiViewFactory> descFactory(new DescriptorImagePointMultiViewFactory(configEstimation.DESC_SIZE, configEstimation.DESC_SCALE_STEP, jmath::degToRad(configEstimation.DESC_ANGLE_STEP), (DescriptorImagePointMultiView::PredictionType)configEstimation.DESC_PREDICTION_TYPE));
		#else
		boost::shared_ptr<DescriptorImagePointFirstViewFactory> descFactory(new DescriptorImagePointFirstViewFactory(configEstimation.DESC_SIZE));
		#endif
		boost::shared_ptr<ImagePointHarrisDetector> harrisDetector(new ImagePointHarrisDetector(configEstimation.HARRIS_CONV_SIZE, configEstimation.HARRIS_TH, configEstimation.HARRIS_EDDGE, configEstimation.PATCH_SIZE, configEstimation.PIX_NOISE, descFactory));
		boost::shared_ptr<ImagePointZnccMatcher> znccMatcher(new ImagePointZnccMatcher(configEstimation.MIN_SCORE, configEstimation.PARTIAL_POSITION, configEstimation.PATCH_SIZE, configEstimation.MAX_SEARCH_SIZE, configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac> dmPt11(new DataManager_ImagePoint_Ransac(harrisDetector, znccMatcher, asGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);

		
		#ifdef HAVE_VIAM
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, rawdata_mutex, 
			configSetup.CAMERA_DEVICE, cv::Size(img_width,img_height), 0, 8, floatOpts[fFreq], intOpts[iTrigger], 
			floatOpts[fShutter], mode, strOpts[sDataPath]));
		senPtr11->setHardwareSensor(hardSen11);
		#else
		if (intOpts[iReplay] & 1)
		{
			hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, rawdata_mutex, cv::Size(img_width,img_height),strOpts[sDataPath]));
			senPtr11->setHardwareSensor(hardSen11);
		}
		#endif
	}
	
	//--- force a first display with empty slam to ensure that all windows are loaded
// std::cout << "SLAM: forcing first initialization display" << std::endl;
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

	if (intOpts[iDispQt] || intOpts[iDispGdhe])
	{
		boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
		(*world)->display_rendered = false;
		display_lock.unlock();
		(*world)->display_condition.notify_all();
// std::cout << "SLAM: now waiting for this display to finish" << std::endl;
		display_lock.lock();
		while(!(*world)->display_rendered) (*world)->display_condition.wait(display_lock);
		display_lock.unlock();
	}

// std::cout << "SLAM: starting slam" << std::endl;

	// Show empty map
//	std::cout << *mapPtr << std::endl;

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
	//if (dataLogger) dataLogger->log();
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
			// std::cout << "\n================================================== " << std::endl;
			// std::cout << *robPtr << std::endl;

			// foreach sensor
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin();
				senIter != robPtr->sensorList().end(); ++senIter)
			{
				sensor_ptr_t senPtr = *senIter;
				//					std::cout << "\n________________________________________________ " << std::endl;
				//					std::cout << *senPtr << std::endl;

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
// cout << "\n************************************************** " << std::endl;
JFR_DEBUG("                 FRAME : " << (*world)->t);
//				std::cout << "Robot: " << *robPtr << std::endl;
//				std::cout << "Pert: " << robPtr->perturbation.P() << "\nPert Jac: " << robPtr->XNEW_pert << "\nState pert: " << robPtr->Q << std::endl;
//				std::cout << "Robot state: " << robPtr->state.x() << std::endl;

				if (intOpts[iReplay] != 2)
				{
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
						dmaPtr->processKnown(senPtr->getRaw());
						dmaPtr->mapManagerPtr()->manage();
						dmaPtr->detectNew(senPtr->getRaw());
					} // foreach dataManager

average_robot_innovation += ublas::norm_2(robPtr->state.x() - robot_prediction);
n_innovation++;

				} else
				{
					robPtr->move_fake(senPtr->getRaw()->timestamp);
				}

			} // for each sensor
		} // for each robot

		// NOW LOOP FOR STATE SPACE - ALL MM
		if (had_data)
		{
JFR_DEBUG("Robot state after corrections " << robPtr1->state.x());
JFR_DEBUG("Robot state stdev after corrections " << sqrt(ublas::matrix_vector_range<jblas::sym_mat_indirect>(robPtr1->state.P(), ublas::range(0, robPtr1->state.P().size1()), ublas::range (0, robPtr1->state.P().size2()))));
			if (robPtr1->dt_or_dx > max_dt) max_dt = robPtr1->dt_or_dx;

			// Output info
//						std::cout << std::endl;
//						std::cout << "dt: " << (int) (1000 * robPtr1->dt_or_dx) << "ms (match "
//						<< total_match_time/1000 << " ms, update " << total_update_time/1000 << "ms). Lmk: [";
//						std::cout << mmPoint->landmarkList().size() << "] ";
//						for (MapManagerAbstract::LandmarkList::iterator lmkIter =
//								mmPoint->landmarkList().begin(); lmkIter
//								!= mmPoint->landmarkList().end(); lmkIter++) {
//							std::cout << (*lmkIter)->id() << " ";
//						}
#if 0
			for (MapAbstract::MapManagerList::iterator mmIter = mapPtr->mapManagerList().begin(); 
				mmIter != mapPtr->mapManagerList().end(); ++mmIter)
			{
				map_manager_ptr_t mapMgr = *mmIter;
				mapMgr->manage();
			}
#endif
// cout << "SLAM: processed a frame: t " << (*world)->t << " display_t " << (*world)->display_t << std::endl;

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

			if ((intOpts[iDispQt] || intOpts[iDispGdhe]) && renderAll)
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
//cout << "SLAM: checking if display has rendered" << std::endl;
			boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
			if ((*world)->display_rendered)
			{
// cout << "SLAM: display has finished, let's bufferize this one" << std::endl;
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


/** ############################################################################
 * #############################################################################
 * Display function
 * ###########################################################################*/

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
// std::cout << "DISPLAY: waiting for data" << std::endl;
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
// std::cout << "DISPLAY: ok data here, let's start!" << std::endl;

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
			
			if (((intOpts[iReplay] & 1) || intOpts[iSimu]) && intOpts[iDump] && (*world)->display_t+1 != 0)
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
// std::cout << "DISPLAY: finished display, marking rendered" << std::endl;
		display_lock.lock();
		(*world)->display_rendered = true;
		display_lock.unlock();
		(*world)->display_condition.notify_all();
		
		if (intOpts[iDispQt]) break; else timer.wait();
	}
}


/** ############################################################################
 * #############################################################################
 * Exit function
 * ###########################################################################*/

void demo_slam01_exit(world_ptr_t *world, boost::thread *thread_main) {
	(*world)->exit(true);
	(*world)->display_condition.notify_all();
// 	std::cout << "EXITING !!!" << std::endl;
	//fputc('\n', stdin);
	thread_main->timed_join(boost::posix_time::milliseconds(500));
}

/** ############################################################################
 * #############################################################################
 * Demo function
 * ###########################################################################*/

void demo_slam01() {
	world_ptr_t worldPtr(new WorldAbstract());

	// deal with the random seed
	rseed = time(NULL);
	if (intOpts[iRandSeed] != 0 && intOpts[iRandSeed] != 1)
		rseed = intOpts[iRandSeed];
	if (!(intOpts[iReplay] & 1) && intOpts[iDump]) {
		std::fstream f((strOpts[sDataPath] + std::string("/rseed.log")).c_str(), std::ios_base::out);
		f << rseed << std::endl;
		f.close();
	}
	else if ((intOpts[iReplay] & 1) && intOpts[iRandSeed] == 1) {
		std::fstream f((strOpts[sDataPath] + std::string("/rseed.log")).c_str(), std::ios_base::in);
		f >> rseed;
		f.close();
	}
	std::cout << "Random seed " << rseed << std::endl;
	rtslam::srand(rseed);

	#ifdef HAVE_MODULE_QDISPLAY
	if (intOpts[iDispQt])
	{
		display::ViewerQt *viewerQt = new display::ViewerQt(8, configEstimation.MAHALANOBIS_TH, false, "data/rendered2D_%02d-%06d.png");
		worldPtr->addDisplayViewer(viewerQt, display::ViewerQt::id());
	}
	#endif
	#ifdef HAVE_MODULE_GDHE
	if (intOpts[iDispGdhe])
	{
		display::ViewerGdhe *viewerGdhe = new display::ViewerGdhe("camera", configEstimation.MAHALANOBIS_TH, "localhost");
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



/** ############################################################################
 * #############################################################################
 * main function
 * ###########################################################################*/

/**
	* Program options:
	* --disp-2d=0/1
	* --disp-3d=0/1
	* --render-all=0/1 (needs --replay 1)
	* --replay=0/1/2/3 (off/on/off no slam/on true time) (needs --data-path)
	* --dump=0/1  (needs --data-path)
	* --rand-seed=0/1/n, 0=generate new one, 1=in replay use the saved one, n=use seed n
	* --pause=0/n 0=don't, n=pause for frames>n (needs --replay 1)
	* --log=0/1 -> log result in text file
	* --verbose=0/1/2/3/4/5 -> Off/Trace/Warning/Debug/VerboseDebug/VeryVerboseDebug
	* --data-path=/mnt/ram/rtslam
	* --config-setup=data/setup.cfg
	* --config-estimation=data/estimation.cfg
	* --help
	* --usage
	* --robot 0=constant vel, 1=inertial
	* --map 0=odometry, 1=global, 2=local/multimap
	* --trigger 0=internal, 1=external mode 1, 2=external mode 0, 3=external mode 14 (PointGrey (Flea) only)
	* --simu 0/1
	* --freq camera frequency in double Hz (with trigger==0/1)
	* --shutter shutter time in double seconds (0=auto); for trigger modes 0,2,3 the value is relative between 0 and 1
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
	floatOpts[fShutter] = 0.0;
	strOpts[sDataPath] = ".";
	strOpts[sConfigSetup] = "#!@";
	strOpts[sConfigEstimation] = "data/estimation.cfg";
	
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
	if (intOpts[iReplay] & 1) mode = 2; else
		if (intOpts[iDump]) mode = 1; else
			mode = 0;
	if (strOpts[sConfigSetup] == "#!@")
	{
		if (intOpts[iReplay] & 1)
			strOpts[sConfigSetup] = strOpts[sDataPath] + "/setup.cfg";
		else
			strOpts[sConfigSetup] = "data/setup.cfg";
	}
	if (strOpts[sConfigSetup][0] == '@' && strOpts[sConfigSetup][1] == '/')
		strOpts[sConfigSetup] = strOpts[sDataPath] + strOpts[sConfigSetup].substr(1);
	if (strOpts[sConfigEstimation][0] == '@' && strOpts[sConfigEstimation][1] == '/')
		strOpts[sConfigEstimation] = strOpts[sDataPath] + strOpts[sConfigEstimation].substr(1);
	if (!(intOpts[iReplay] & 1) && intOpts[iDump]) boost::filesystem::copy_file(strOpts[sConfigSetup], strOpts[sDataPath] + "/setup.cfg");
	#ifndef HAVE_MODULE_QDISPLAY
	intOpts[iDispQt] = 0;
	#endif
	#ifndef HAVE_MODULE_GDHE
	intOpts[iDispGdhe] = 0;
	#endif

	try {
		std::cout << "Loading config files " << strOpts[sConfigSetup] << " and " << strOpts[sConfigEstimation] << std::endl;
		configSetup.load(strOpts[sConfigSetup]);
		configEstimation.load(strOpts[sConfigEstimation]);
		
		demo_slam01();
	} catch (kernel::Exception &e) { std::cout << e.what(); return 1; }
}



/** ############################################################################
 * #############################################################################
 * Config file loading
 * ###########################################################################*/

#define KeyValueFile_getItem(k) keyValueFile.getItem(#k, k);
#define KeyValueFile_setItem(k) keyValueFile.setItem(#k, k);


void ConfigSetup::loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile)
{
	KeyValueFile_getItem(SENSOR_POSE_CONSTVEL);
	KeyValueFile_getItem(SENSOR_POSE_INERTIAL);
	
	KeyValueFile_getItem(CAMERA_DEVICE);
	KeyValueFile_getItem(IMG_WIDTH);
	KeyValueFile_getItem(IMG_HEIGHT);
	KeyValueFile_getItem(INTRINSIC);
	KeyValueFile_getItem(DISTORTION);
	
	KeyValueFile_getItem(IMG_WIDTH_SIMU);
	KeyValueFile_getItem(IMG_HEIGHT_SIMU);
	KeyValueFile_getItem(INTRINSIC_SIMU);
	KeyValueFile_getItem(DISTORTION_SIMU);
	
	KeyValueFile_getItem(UNCERT_VLIN);
	KeyValueFile_getItem(UNCERT_VANG);
	KeyValueFile_getItem(PERT_VLIN);
	KeyValueFile_getItem(PERT_VANG);
	
	KeyValueFile_getItem(MTI_DEVICE);
	KeyValueFile_getItem(ACCELERO_FULLSCALE);
	KeyValueFile_getItem(ACCELERO_NOISE);
	KeyValueFile_getItem(GYRO_FULLSCALE);
	KeyValueFile_getItem(GYRO_NOISE);
	
	KeyValueFile_getItem(UNCERT_GRAVITY);
	KeyValueFile_getItem(UNCERT_ABIAS);
	KeyValueFile_getItem(UNCERT_WBIAS);
	KeyValueFile_getItem(PERT_AERR);
	KeyValueFile_getItem(PERT_WERR);
	KeyValueFile_getItem(PERT_RANWALKACC);
	KeyValueFile_getItem(PERT_RANWALKGYRO);
	
	KeyValueFile_getItem(IMU_TIMESTAMP_CORRECTION);
	
	KeyValueFile_getItem(SIMU_IMU_TIMESTAMP_CORRECTION);
	KeyValueFile_getItem(SIMU_IMU_FREQ);
	KeyValueFile_getItem(SIMU_IMU_GRAVITY);
	KeyValueFile_getItem(SIMU_IMU_GYR_BIAS);
	KeyValueFile_getItem(SIMU_IMU_GYR_BIAS_NOISESTD);
	KeyValueFile_getItem(SIMU_IMU_GYR_GAIN);
	KeyValueFile_getItem(SIMU_IMU_GYR_GAIN_NOISESTD);
	KeyValueFile_getItem(SIMU_IMU_RANDWALKGYR_FACTOR);
	KeyValueFile_getItem(SIMU_IMU_ACC_BIAS);
	KeyValueFile_getItem(SIMU_IMU_ACC_BIAS_NOISESTD);
	KeyValueFile_getItem(SIMU_IMU_ACC_GAIN);
	KeyValueFile_getItem(SIMU_IMU_ACC_GAIN_NOISESTD);
	KeyValueFile_getItem(SIMU_IMU_RANDWALKACC_FACTOR);
}

void ConfigSetup::saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile)
{
	KeyValueFile_setItem(SENSOR_POSE_CONSTVEL);
	KeyValueFile_setItem(SENSOR_POSE_INERTIAL);
	
	KeyValueFile_setItem(CAMERA_DEVICE);
	KeyValueFile_setItem(IMG_WIDTH);
	KeyValueFile_setItem(IMG_HEIGHT);
	KeyValueFile_setItem(INTRINSIC);
	KeyValueFile_setItem(DISTORTION);
	
	KeyValueFile_setItem(IMG_WIDTH_SIMU);
	KeyValueFile_setItem(IMG_HEIGHT_SIMU);
	KeyValueFile_setItem(INTRINSIC_SIMU);
	KeyValueFile_setItem(DISTORTION_SIMU);
	
	KeyValueFile_setItem(UNCERT_VLIN);
	KeyValueFile_setItem(UNCERT_VANG);
	KeyValueFile_setItem(PERT_VLIN);
	KeyValueFile_setItem(PERT_VANG);
	
	KeyValueFile_setItem(MTI_DEVICE);
	KeyValueFile_setItem(ACCELERO_FULLSCALE);
	KeyValueFile_setItem(ACCELERO_NOISE);
	KeyValueFile_setItem(GYRO_FULLSCALE);
	KeyValueFile_setItem(GYRO_NOISE);
	
	KeyValueFile_setItem(UNCERT_GRAVITY);
	KeyValueFile_setItem(UNCERT_ABIAS);
	KeyValueFile_setItem(UNCERT_WBIAS);
	KeyValueFile_setItem(PERT_AERR);
	KeyValueFile_setItem(PERT_WERR);
	KeyValueFile_setItem(PERT_RANWALKACC);
	KeyValueFile_setItem(PERT_RANWALKGYRO);
	
	KeyValueFile_setItem(IMU_TIMESTAMP_CORRECTION);
	
	KeyValueFile_setItem(SIMU_IMU_TIMESTAMP_CORRECTION);
	KeyValueFile_setItem(SIMU_IMU_FREQ);
	KeyValueFile_setItem(SIMU_IMU_GRAVITY);
	KeyValueFile_setItem(SIMU_IMU_GYR_BIAS);
	KeyValueFile_setItem(SIMU_IMU_GYR_BIAS_NOISESTD);
	KeyValueFile_setItem(SIMU_IMU_GYR_GAIN);
	KeyValueFile_setItem(SIMU_IMU_GYR_GAIN_NOISESTD);
	KeyValueFile_setItem(SIMU_IMU_RANDWALKGYR_FACTOR);
	KeyValueFile_setItem(SIMU_IMU_ACC_BIAS);
	KeyValueFile_setItem(SIMU_IMU_ACC_BIAS_NOISESTD);
	KeyValueFile_setItem(SIMU_IMU_ACC_GAIN);
	KeyValueFile_setItem(SIMU_IMU_ACC_GAIN_NOISESTD);
	KeyValueFile_setItem(SIMU_IMU_RANDWALKACC_FACTOR);
}

void ConfigEstimation::loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile)
{
	KeyValueFile_getItem(CORRECTION_SIZE);
	
	KeyValueFile_getItem(MAP_SIZE);
	KeyValueFile_getItem(PIX_NOISE);
	KeyValueFile_getItem(PIX_NOISE_SIMUFACTOR);
	
	KeyValueFile_getItem(D_MIN);
	KeyValueFile_getItem(REPARAM_TH);
	
	KeyValueFile_getItem(GRID_HCELLS);
	KeyValueFile_getItem(GRID_VCELLS);
	KeyValueFile_getItem(GRID_MARGIN);
	KeyValueFile_getItem(GRID_SEPAR);
	
	KeyValueFile_getItem(RELEVANCE_TH);
	KeyValueFile_getItem(MAHALANOBIS_TH);
	KeyValueFile_getItem(N_UPDATES_TOTAL);
	KeyValueFile_getItem(N_UPDATES_RANSAC);
	KeyValueFile_getItem(N_INIT);
	KeyValueFile_getItem(N_RECOMP_GAINS);
	KeyValueFile_getItem(RANSAC_LOW_INNOV);
	
	KeyValueFile_getItem(RANSAC_NTRIES);
	
	KeyValueFile_getItem(HARRIS_CONV_SIZE);
	KeyValueFile_getItem(HARRIS_TH);
	KeyValueFile_getItem(HARRIS_EDDGE);
	
	KeyValueFile_getItem(DESC_SIZE);
	KeyValueFile_getItem(DESC_SCALE_STEP);
	KeyValueFile_getItem(DESC_ANGLE_STEP);
	KeyValueFile_getItem(DESC_PREDICTION_TYPE);
	
	KeyValueFile_getItem(PATCH_SIZE);
	KeyValueFile_getItem(MAX_SEARCH_SIZE);
	KeyValueFile_getItem(KILL_SEARCH_SIZE);
	KeyValueFile_getItem(MATCH_TH);
	KeyValueFile_getItem(MIN_SCORE);
	KeyValueFile_getItem(PARTIAL_POSITION);
}

void ConfigEstimation::saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile)
{
	KeyValueFile_setItem(CORRECTION_SIZE);
	
	KeyValueFile_setItem(MAP_SIZE);
	KeyValueFile_setItem(PIX_NOISE);
	KeyValueFile_setItem(PIX_NOISE_SIMUFACTOR);
	
	KeyValueFile_setItem(D_MIN);
	KeyValueFile_setItem(REPARAM_TH);
	
	KeyValueFile_setItem(GRID_HCELLS);
	KeyValueFile_setItem(GRID_VCELLS);
	KeyValueFile_setItem(GRID_MARGIN);
	KeyValueFile_setItem(GRID_SEPAR);
	
	KeyValueFile_setItem(RELEVANCE_TH);
	KeyValueFile_setItem(MAHALANOBIS_TH);
	KeyValueFile_setItem(N_UPDATES_TOTAL);
	KeyValueFile_setItem(N_UPDATES_RANSAC);
	KeyValueFile_setItem(N_INIT);
	KeyValueFile_setItem(N_RECOMP_GAINS);
	KeyValueFile_setItem(RANSAC_LOW_INNOV);
	
	KeyValueFile_setItem(RANSAC_NTRIES);
	
	KeyValueFile_setItem(HARRIS_CONV_SIZE);
	KeyValueFile_setItem(HARRIS_TH);
	KeyValueFile_setItem(HARRIS_EDDGE);
	
	KeyValueFile_setItem(DESC_SIZE);
	KeyValueFile_setItem(DESC_SCALE_STEP);
	KeyValueFile_setItem(DESC_ANGLE_STEP);
	KeyValueFile_setItem(DESC_PREDICTION_TYPE);
	
	KeyValueFile_setItem(PATCH_SIZE);
	KeyValueFile_setItem(MAX_SEARCH_SIZE);
	KeyValueFile_setItem(KILL_SEARCH_SIZE);
	KeyValueFile_setItem(MATCH_TH);
	KeyValueFile_setItem(MIN_SCORE);
	KeyValueFile_setItem(PARTIAL_POSITION);
}
