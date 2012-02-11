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
//#define EVENT_BASED_RAW 1 // always enabled now

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
//#define MULTIVIEW_DESCRIPTOR 1 // moved in config file

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


/*
 * STATUS: in progress, do not use for now
 * This uses HDseg powered Segment based slam instead of the usual point based slam.
 * 0 use points
 * 1 use segments
 * 2 use both sgments and points
 */
#define SEGMENT_BASED 0
#if SEGMENT_BASED>1
	#define SEGMENT_NOISE_FACTOR 5
#else
	#define SEGMENT_NOISE_FACTOR 1
#endif

#if SEGMENT_BASED
	#ifndef HAVE_MODULE_DSEG
	#error "dseg module is required for segment based slam"
	#endif
#endif


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
#include "kernel/threads.hpp"
#include "jmath/random.hpp"
#include "jmath/matlab.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/rawProcessors.hpp"
#include "rtslam/rawSegProcessors.hpp"
//#include "rtslam/robotOdometry.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/robotInertial.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/sensorAbsloc.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"
//#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationMakers.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/activeSegmentSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/descriptorSeg.hpp"
#include "rtslam/dataManagerOnePointRansac.hpp"
#include "rtslam/sensorManager.hpp"

#include "rtslam/hardwareSensorCameraFirewire.hpp"
#include "rtslam/hardwareEstimatorMti.hpp"
#include "rtslam/hardwareSensorGpsGenom.hpp"
#include "rtslam/hardwareSensorMocap.hpp"

#include "rtslam/display_qt.hpp"
#include "rtslam/display_gdhe.hpp"

#include "rtslam/simuRawProcessors.hpp"
#include "rtslam/hardwareSensorAdhocSimulator.hpp"
#include "rtslam/hardwareEstimatorInertialAdhocSimulator.hpp"
#include "rtslam/exporterSocket.hpp"


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


typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinhole, LandmarkEuclideanPoint,
	 AppearanceImagePoint, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> PinholeEucpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinhole, LandmarkEuclideanPoint,
	 simu::AppearanceSimu, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> PinholeEucpSimuObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinhole, LandmarkAnchoredHomogeneousPoint,
	AppearanceImagePoint, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH> PinholeAhpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinhole, LandmarkAnchoredHomogeneousPoint,
	simu::AppearanceSimu, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH> PinholeAhpSimuObservationMaker;

typedef DataManagerOnePointRansac<RawImage, SensorPinhole, FeatureImagePoint, image::ConvexRoi, ActiveSearchGrid, ImagePointHarrisDetector, ImagePointZnccMatcher> DataManager_ImagePoint_Ransac;
typedef DataManagerOnePointRansac<simu::RawSimu, SensorPinhole, simu::FeatureSimu, image::ConvexRoi, ActiveSearchGrid, simu::DetectorSimu<image::ConvexRoi>, simu::MatcherSimu<image::ConvexRoi> > DataManager_ImagePoint_Ransac_Simu;

#if SEGMENT_BASED
typedef SegmentObservationMaker<ObservationPinHoleAnchoredHomogeneousPointsLine, SensorPinhole, LandmarkAnchoredHomogeneousPointsLine,
   AppearanceImageSegment, SensorAbstract::PINHOLE, LandmarkAbstract::LINE_AHPL> PinholeAhplObservationMaker;
typedef SegmentObservationMaker<ObservationPinHoleAnchoredHomogeneousPointsLine, SensorPinhole, LandmarkAnchoredHomogeneousPointsLine,
	simu::AppearanceSimu, SensorAbstract::PINHOLE, LandmarkAbstract::LINE_AHPL> PinholeAhplSimuObservationMaker;

typedef DataManagerOnePointRansac<RawImage, SensorPinhole, FeatureImageSegment, image::ConvexRoi, ActiveSegmentSearchGrid, HDsegDetector, DsegMatcher> DataManager_ImageSeg_Test;
typedef DataManagerOnePointRansac<simu::RawSimu, SensorPinhole, simu::FeatureSimu, image::ConvexRoi, ActiveSegmentSearchGrid, simu::DetectorSimu<image::ConvexRoi>, simu::MatcherSimu<image::ConvexRoi> > DataManager_Segment_Ransac_Simu;
#endif

int mode = 0;
time_t rseed;


/** ############################################################################
 * #############################################################################
 * program parameters
 * ###########################################################################*/

enum { iDispQt = 0, iDispGdhe, iRenderAll, iReplay, iDump, iRandSeed, iPause, iVerbose, iMap, iRobot, iCamera, iTrigger, iGps, iSimu, iExport, nIntOpts };
int intOpts[nIntOpts] = {0};
const int nFirstIntOpt = 0, nLastIntOpt = nIntOpts-1;

enum { fFreq = 0, fShutter, fHeading, nFloatOpts };
double floatOpts[nFloatOpts] = {0.0};
const int nFirstFloatOpt = nIntOpts, nLastFloatOpt = nIntOpts+nFloatOpts-1;

enum { sDataPath = 0, sConfigSetup, sConfigEstimation, sLog, nStrOpts };
std::string strOpts[nStrOpts];
const int nFirstStrOpt = nIntOpts+nFloatOpts, nLastStrOpt = nIntOpts+nFloatOpts+nStrOpts-1;

enum { bHelp = 0, bUsage, nBreakingOpts };
const int nFirstBreakingOpt = nIntOpts+nFloatOpts+nStrOpts, nLastBreakingOpt = nIntOpts+nFloatOpts+nStrOpts+nBreakingOpts-1;

/// !!WARNING!! be careful that options are in the same order above and below

#ifndef GENOM

struct option long_options[] = {
	// int options
	{"disp-2d", 2, 0, 0},
	{"disp-3d", 2, 0, 0},
	{"render-all", 2, 0, 0},
	{"replay", 2, 0, 0},
	{"dump", 2, 0, 0},
	{"rand-seed", 2, 0, 0},
	{"pause", 2, 0, 0},
	{"verbose", 2, 0, 0},
	{"map", 2, 0, 0},
	{"robot", 2, 0, 0}, // should be in config file
	{"camera", 2, 0, 0},
	{"trigger", 2, 0, 0}, // should be in config file
	{"gps", 2, 0, 0},
	{"simu", 2, 0, 0},
	{"export", 2, 0, 0},
	// double options
	{"freq", 2, 0, 0}, // should be in config file
	{"shutter", 2, 0, 0}, // should be in config file
	{"heading", 2, 0, 0},
	// string options
	{"data-path", 1, 0, 0},
	{"config-setup", 1, 0, 0},
	{"config-estimation", 1, 0, 0},
	{"log", 1, 0, 0},
	// breaking options
	{"help",0,0,0},
	{"usage",0,0,0},
};

#endif

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
	jblas::vec6 SENSOR_POSE_CONSTVEL; /// camera pose in constant velocity (x,y,z,roll,pitch,yaw) (m,deg)
	jblas::vec6 SENSOR_POSE_INERTIAL; /// camera pose in inertial (x,y,z,roll,pitch,yaw) (m,deg)
	jblas::vec6 GPS_POSE; /// GPS pose (x,y,z,roll,pitch,yaw) (m,deg)
	jblas::vec6 ROBOT_POSE; /// the transformation between the slam robot (the main sensor, camera or imu) and the real robot = pose of the real robot in the slam robot frame, just like the other sensors

	unsigned CAMERA_TYPE;      /// camera type (0 = firewire, 1 = firewire format7, 2 = USB)
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
	double PERT_AERR;        /// noise stdev coeff of accelerometers, for testing purpose (% of ACCELERO_NOISE)
	double PERT_WERR;        /// noise stdev coeff of gyrometers, for testing purpose (% of GYRO_NOISE)
	double PERT_RANWALKACC;  /// IMU a_bias random walk (m/s2 per sqrt(s))
	double PERT_RANWALKGYRO; /// IMU w_bias random walk (rad/s per sqrt(s))

	double UNCERT_HEADING;   /// initial heading uncertainty
	double UNCERT_ATTITUDE;   /// initial attitude angles uncertainty
	
	double IMU_TIMESTAMP_CORRECTION; /// correction to add to the IMU timestamp for synchronization (s)
	double GPS_TIMESTAMP_CORRECTION; /// correction to add to the GPS timestamp for synchronization (s)
	
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
	bool MULTIVIEW_DESCRIPTOR; /// whether use or not the multiview descriptor
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

world_ptr_t worldPtr;
boost::scoped_ptr<kernel::DataLogger> dataLogger;
sensor_manager_ptr_t sensorManager;
boost::shared_ptr<ExporterAbstract> exporter;
#ifdef HAVE_MODULE_QDISPLAY
display::ViewerQt *viewerQt = NULL;
#endif
#ifdef HAVE_MODULE_GDHE
display::ViewerGdhe *viewerGdhe = NULL;
#endif
kernel::VariableCondition<int> rawdata_condition(0);


void demo_slam_init()
{ try {
	// preprocess options
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
	if (intOpts[iReplay] & 1) intOpts[iExport] = 0;
	if (strOpts[sConfigSetup][0] == '@' && strOpts[sConfigSetup][1] == '/')
		strOpts[sConfigSetup] = strOpts[sDataPath] + strOpts[sConfigSetup].substr(1);
	if (strOpts[sConfigEstimation][0] == '@' && strOpts[sConfigEstimation][1] == '/')
		strOpts[sConfigEstimation] = strOpts[sDataPath] + strOpts[sConfigEstimation].substr(1);
	if (!(intOpts[iReplay] & 1) && intOpts[iDump])
	{
		boost::filesystem::remove(strOpts[sDataPath] + "/setup.cfg");
		boost::filesystem::remove(strOpts[sDataPath] + "/setup.cfg.maybe");
		if (intOpts[iReplay] == 2)
			boost::filesystem::copy_file(strOpts[sConfigSetup], strOpts[sDataPath] + "/setup.cfg.maybe"/*, boost::filesystem::copy_option::overwrite_if_exists*/);
		else
			boost::filesystem::copy_file(strOpts[sConfigSetup], strOpts[sDataPath] + "/setup.cfg"/*, boost::filesystem::copy_option::overwrite_if_exists*/);
	}
	#ifndef HAVE_MODULE_QDISPLAY
	intOpts[iDispQt] = 0;
	#endif
	#ifndef HAVE_MODULE_GDHE
	intOpts[iDispGdhe] = 0;
	#endif

	if (strOpts[sLog].size() == 1)
	{
		if (strOpts[sLog][0] == '0') strOpts[sLog] = ""; else
		if (strOpts[sLog][0] == '1') strOpts[sLog] = "rtslam.log";
	}
		
		
	// init
	worldPtr.reset(new WorldAbstract());

	std::cout << "Loading config files " << strOpts[sConfigSetup] << " and " << strOpts[sConfigEstimation] << std::endl;
	configSetup.load(strOpts[sConfigSetup]);
	configEstimation.load(strOpts[sConfigEstimation]);
	
	// deal with the random seed
	rseed = jmath::get_srand();
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
	intOpts[iRandSeed] = rseed;
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
	
	if (!strOpts[sLog].empty())
	{
		dataLogger.reset(new kernel::DataLogger(strOpts[sDataPath] + "/" + strOpts[sLog]));
		dataLogger->writeCurrentDate();
		dataLogger->writeNewLine();
#ifndef GENOM
// FIXME do it for genom
		// write options to log
		std::ostringstream oss;
		for(int i = 0; i < nIntOpts; ++i)
			{ oss << long_options[i+nFirstIntOpt].name << " = " << intOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		for(int i = 0; i < nFloatOpts; ++i)
			{ oss << long_options[i+nFirstFloatOpt].name << " = " << floatOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		for(int i = 0; i < nStrOpts; ++i)
			{ oss << long_options[i+nFirstStrOpt].name << " = " << strOpts[i]; dataLogger->writeComment(oss.str()); oss.str(""); }
		dataLogger->writeNewLine();
#endif
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
#if SEGMENT_BASED
	if (intOpts[iSimu] != 0)
	{
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhplSimuObservationMaker(
			configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE, 30, 0.5, 0.5, configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
	} else
	{
		obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhplObservationMaker(
			configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE, 30, 0.5, 0.5, configEstimation.D_MIN, configEstimation.PATCH_SIZE)));
	}
#endif
#if SEGMENT_BASED != 1
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
#endif

	// ---------------------------------------------------------------------------
	// --- INIT ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : 1 map and map-manager, 2 robs, 3 sens and data-manager.

	// 1. Create maps.
	map_ptr_t mapPtr(new MapAbstract(configEstimation.MAP_SIZE));
	mapPtr->linkToParentWorld(worldPtr);
	
   // 1b. Create map manager.
	landmark_factory_ptr_t pointLmkFactory;
	landmark_factory_ptr_t segLmkFactory;
#if SEGMENT_BASED
   segLmkFactory.reset(new LandmarkFactory<LandmarkAnchoredHomogeneousPointsLine, LandmarkAnchoredHomogeneousPointsLine>());
#endif
#if SEGMENT_BASED != 1
   pointLmkFactory.reset(new LandmarkFactory<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint>());
#endif
	map_manager_ptr_t mmPoint;
	map_manager_ptr_t mmSeg;
	switch(intOpts[iMap])
	{
		case 0: { // odometry
			if(pointLmkFactory != NULL)
				mmPoint.reset(new MapManagerOdometry(pointLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			if(segLmkFactory != NULL)
				mmSeg.reset(new MapManagerOdometry(segLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			break;
		}
		case 1: { // global
			if(pointLmkFactory != NULL)
				mmPoint.reset(new MapManagerGlobal(pointLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE, 30, 0.5, 0.5));
			if(segLmkFactory != NULL)
				mmSeg.reset(new MapManagerGlobal(segLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE, 30, 0.5, 0.5));
			break;
		}
		case 2: { // local/multimap
			if(pointLmkFactory != NULL)
				mmPoint.reset(new MapManagerLocal(pointLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			if(segLmkFactory != NULL)
				mmSeg.reset(new MapManagerLocal(segLmkFactory, configEstimation.REPARAM_TH, configEstimation.KILL_SEARCH_SIZE));
			break;	
		}
	}
	if(mmPoint != NULL)
		mmPoint->linkToParentMap(mapPtr);
	if(mmSeg != NULL)
		mmSeg->linkToParentMap(mapPtr);

	// simulation environment
	boost::shared_ptr<simu::AdhocSimulator> simulator;
	if (intOpts[iSimu] != 0)
	{
		simulator.reset(new simu::AdhocSimulator());
		#if SEGMENT_BASED
			jblas::vec11 pose;
		#else
			jblas::vec3 pose;
		#endif

		const int maxnpoints = 1000;
		int npoints = 0;
		#if SEGMENT_BASED
			double points[maxnpoints][11];
		#else
			double points[maxnpoints][3];
		#endif

		switch (intOpts[iSimu]/10)
		{
		#if SEGMENT_BASED
			case 1: {
				// 3D cube
				const int npoints_ = 36; npoints = npoints_;
				double tmp[npoints_][11] = {
					{0,0,0,5,-1,-1,1,5,-1, 1,1}, {0,0,0,5,1,1,1,5,1,-1,1}, {0,0,0,5,-1,1,1,5, 1,1,1}, {0,0,0,5,-1,-1,1,5,1,-1,1},
					{0,0,0,3,-1,-1,1,3,-1, 1,1}, {0,0,0,3,1,1,1,3,1,-1,1}, {0,0,0,3,-1,1,1,3, 1,1,1}, {0,0,0,3,-1,-1,1,3,1,-1,1},
					{0,0,0,5,-1,-1,1,3,-1,-1,1}, {0,0,0,5,1,1,1,3,1, 1,1}, {0,0,0,5,-1,1,1,3,-1,1,1}, {0,0,0,5, 1,-1,1,3,1,-1,1},
					{0,0,0,5,-4,-1,1,5,-4, 1,1}, {0,0,0,5,-2,1,1,5,-2,-1,1}, {0,0,0,5,-4,1,1,5,-2,1,1}, {0,0,0,5,-4,-1,1,5,-2,-1,1},
					{0,0,0,3,-4,-1,1,3,-4, 1,1}, {0,0,0,3,-2,1,1,3,-2,-1,1}, {0,0,0,3,-4,1,1,3,-2,1,1}, {0,0,0,3,-4,-1,1,3,-2,-1,1},
					{0,0,0,5,-4,-1,1,3,-4,-1,1}, {0,0,0,5,-2,1,1,3,-2, 1,1}, {0,0,0,5,-4,1,1,3,-4,1,1}, {0,0,0,5,-2,-1,1,3,-2,-1,1},
					{0,0,0,5, 2,-1,1,5, 2, 1,1}, {0,0,0,5,4,1,1,5,4,-1,1}, {0,0,0,5,2,1,1,5, 4,1,1}, {0,0,0,5, 2,-1,1,5,4,-1,1},
					{0,0,0,3, 2,-1,1,3, 2, 1,1}, {0,0,0,3,4,1,1,3,4,-1,1}, {0,0,0,3,2,1,1,3, 4,1,1}, {0,0,0,3, 2,-1,1,3,4,-1,1},
					{0,0,0,5, 2,-1,1,3, 2,-1,1}, {0,0,0,5,4,1,1,3,4, 1,1}, {0,0,0,5,2,1,1,3, 2,1,1}, {0,0,0,5, 4,-1,1,3,4,-1,1}
				};
				memcpy(points, tmp, npoints*11*sizeof(double));
				break;
			}
			case 2: {
				// 2D square
				const int npoints_ = 4; npoints = npoints_;
				double tmp[npoints_][11] = {
					{0,0,0,5,-1,-1,1,5,-1,1,1}, {0,0,0,5,1,1,1,5,1,-1,1}, {0,0,0,5,-1,1,1,5,1,1,1}, {0,0,0,5,-1,-1,1,5,1,-1,1}
				};
				memcpy(points, tmp, npoints*11*sizeof(double));
				break;
			}
			case 3: {
				// almost 2D square
				const int npoints_ = 4; npoints = npoints_;
				double tmp[npoints_][11] = {
					{0,0,0,4,-1,-1,1,4,-1,1,1}, {0,0,0,4,1,1,1,4,1,-1,1}, {0,0,0,5,-1,1,1,5,1,1,1}, {0,0,0,5,-1,-1,1,5,1,-1,1}
				};
				memcpy(points, tmp, npoints*11*sizeof(double));
				break;
			}
			default: npoints = 0;
		#else
			case 1: {
				// 3D regular grid
				const int npoints_ = 3*11*13; npoints = npoints_;
				for(int i = 0, z = -1; z <= 1; ++z) for(int y = -3; y <= 7; ++y) for(int x = -6; x <= 6; ++x, ++i)
					{ points[i][0] = x*1.0; points[i][1] = y*1.0; points[i][2] = z*1.0; }
				break;
			}
			case 2: {
				// 2D square
				const int npoints_ = 5; npoints = npoints_;
				double tmp[npoints_][3] = { {5,-1,-1}, {5,-1,1}, {5,1,1}, {5,1,-1}, {5,0,0} };
				memcpy(points, tmp, npoints*3*sizeof(double));
				break;
			}
			case 3: {
				// almost 2D square
				const int npoints_ = 5; npoints = npoints_;
				double tmp[npoints_][3] = { {5,-1,-1}, {5,-1,1}, {5,1,1}, {5,1,-1}, {4,0,0} };
				memcpy(points, tmp, npoints*3*sizeof(double));
				break;
			}
		case 4: {
			// far 3D regular grid
			const int npoints_ = 3*11*13; npoints = npoints_;
			for(int i = 0, z = -1; z <= 1; ++z) for(int y = -3; y <= 7; ++y) for(int x = -6; x <= 6; ++x, ++i)
				{ points[i][0] = x*1.0+100; points[i][1] = y*10.0; points[i][2] = z*10.0; }
			break;
		}
		
			default: npoints = 0;
		#endif
		}
		
		// add landmarks
		for(int i = 0; i < npoints; ++i)
		{
			pose(0) = points[i][0]; pose(1) = points[i][1]; pose(2) = points[i][2];
		#if !SEGMENT_BASED
			simu::Landmark *lmk = new simu::Landmark(LandmarkAbstract::POINT, pose);
		#else
			pose(3) = points[i][3]; pose(4) = points[i][4]; pose(5) = points[i][5];
			pose(6) = points[i][6]; pose(7) = points[i][7]; pose(8) = points[i][8];
			pose(9) = points[i][9]; pose(10) = points[i][10];
			simu::Landmark *lmk = new simu::Landmark(LandmarkAbstract::LINE, pose);
		#endif
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
			//hardEst1_->setUseForInit(true);
			//hardEst1_->setNeedInit(true);
			//hardEst1_->start();
			hardEst1 = hardEst1_;
		}
		robPtr1_->setHardwareEstimator(hardEst1);

		robPtr1 = robPtr1_;
	}

	robPtr1->linkToParentMap(mapPtr);
	robPtr1->pose.x(quaternion::originFrame());
	robPtr1->setPoseStd(0,0,0, 0,0,floatOpts[fHeading], 
	                    0,0,0, configSetup.UNCERT_ATTITUDE,configSetup.UNCERT_ATTITUDE,configSetup.UNCERT_HEADING);
	robPtr1->robot_pose = configSetup.ROBOT_POSE;
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
		
			// horiz loop with rotation (always goes forward)
			case 5: {
				double VEL = 0.5;
				rob->addWaypoint(0,0,0, 0,0,0, VEL/5,0,0, 0,0,0);
				rob->addWaypoint(1,0,0, 0,0,0, VEL,0,0, 0,0,0);
				rob->addWaypoint(3,2,0, 1*M_PI/2,0,0, 0,VEL,0, 100,0,0);
				rob->addWaypoint(1,4,0, 2*M_PI/2,0,0, -VEL,0,0, 0,0,0);
				rob->addWaypoint(-1,4,0, 2*M_PI/2,0,0, -VEL,0,0, 0,0,0);
				rob->addWaypoint(-3,2,0, 3*M_PI/2,0,0, 0,-VEL,0, 100,0,0);
				rob->addWaypoint(-1,0,0, 4*M_PI/2,0,0, VEL,0,0, 0,0,0);
				rob->addWaypoint(0,0,0, 4*M_PI/2,0,0, 0,0,0, 0,0,0);
				break;
			}
		
			// straight line
			case 6: {
				double VEL = 0.5;
				rob->addWaypoint(0,0,0, 0,0,0, 0,0,0, 0,0,0);
				rob->addWaypoint(1,0,0, 0,0,0, VEL,0,0, 0,0,0);
				rob->addWaypoint(20,0,0, 0,0,0, VEL,0,0, 0,0,0);
				break;
			}
		}

		simulator->addRobot(rob);
		
	}
	
	
	// 3. Create sensors.
	if (intOpts[iCamera])
	{
		pinhole_ptr_t senPtr11(new SensorPinhole(robPtr1, MapObject::UNFILTERED));
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
			#if SEGMENT_BASED
				simulator->addObservationModel(robPtr1->id(), senPtr11->id(), LandmarkAbstract::LINE, new ObservationModelPinHoleAnchoredHomogeneousPointsLine(senPtr11));
			#endif
		}

		// 3b. Create data manager.
		boost::shared_ptr<ActiveSearchGrid> asGrid(new ActiveSearchGrid(img_width, img_height, configEstimation.GRID_HCELLS, configEstimation.GRID_VCELLS, configEstimation.GRID_MARGIN, configEstimation.GRID_SEPAR));
		 boost::shared_ptr<ActiveSegmentSearchGrid> assGrid(new ActiveSegmentSearchGrid(img_width, img_height, configEstimation.GRID_HCELLS, configEstimation.GRID_VCELLS, configEstimation.GRID_MARGIN, configEstimation.GRID_SEPAR));

		#if RANSAC_FIRST
		 int ransac_ntries = configEstimation.RANSAC_NTRIES;
		#else
		int ransac_ntries = 0;
		 #endif

		if (intOpts[iSimu] != 0)
		{
			#if SEGMENT_BASED
				boost::shared_ptr<simu::DetectorSimu<image::ConvexRoi> > detector(new simu::DetectorSimu<image::ConvexRoi>(LandmarkAbstract::LINE, 4, configEstimation.PATCH_SIZE, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));
				boost::shared_ptr<simu::MatcherSimu<image::ConvexRoi> > matcher(new simu::MatcherSimu<image::ConvexRoi>(LandmarkAbstract::LINE, 4, configEstimation.PATCH_SIZE, configEstimation.MAX_SEARCH_SIZE, configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));

				boost::shared_ptr<DataManager_Segment_Ransac_Simu> dmPt11(new DataManager_Segment_Ransac_Simu(detector, matcher, assGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

				dmPt11->linkToParentSensorSpec(senPtr11);
				dmPt11->linkToParentMapManager(mmPoint);
				dmPt11->setObservationFactory(obsFact);

				hardware::hardware_sensorext_ptr_t hardSen11(new hardware::HardwareSensorAdhocSimulator(rawdata_condition, floatOpts[fFreq], simulator, robPtr1->id(), senPtr11->id()));
				senPtr11->setHardwareSensor(hardSen11);
			#else
				boost::shared_ptr<simu::DetectorSimu<image::ConvexRoi> > detector(new simu::DetectorSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, configEstimation.PATCH_SIZE, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));
				boost::shared_ptr<simu::MatcherSimu<image::ConvexRoi> > matcher(new simu::MatcherSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, configEstimation.PATCH_SIZE, configEstimation.MAX_SEARCH_SIZE, configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE, configEstimation.PIX_NOISE*configEstimation.PIX_NOISE_SIMUFACTOR));

				boost::shared_ptr<DataManager_ImagePoint_Ransac_Simu> dmPt11(new DataManager_ImagePoint_Ransac_Simu(detector, matcher, asGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

				dmPt11->linkToParentSensorSpec(senPtr11);
				dmPt11->linkToParentMapManager(mmPoint);
				dmPt11->setObservationFactory(obsFact);

				hardware::hardware_sensorext_ptr_t hardSen11(new hardware::HardwareSensorAdhocSimulator(rawdata_condition, floatOpts[fFreq], simulator, robPtr1->id(), senPtr11->id()));
				senPtr11->setHardwareSensor(hardSen11);
			#endif
		} else
		 {
				boost::shared_ptr<DescriptorFactoryAbstract> pointDescFactory;
				boost::shared_ptr<DescriptorFactoryAbstract> segDescFactory;
				#if SEGMENT_BASED

					 if (configEstimation.MULTIVIEW_DESCRIPTOR)
					segDescFactory.reset(new DescriptorImageSegMultiViewFactory(configEstimation.DESC_SIZE, configEstimation.DESC_SCALE_STEP, jmath::degToRad(configEstimation.DESC_ANGLE_STEP), (DescriptorImageSegMultiView::PredictionType)configEstimation.DESC_PREDICTION_TYPE));
					 else
					segDescFactory.reset(new DescriptorImageSegFirstViewFactory(configEstimation.DESC_SIZE));

				boost::shared_ptr<HDsegDetector> hdsegDetector(new HDsegDetector(configEstimation.PATCH_SIZE, 3,configEstimation.PIX_NOISE*SEGMENT_NOISE_FACTOR,segDescFactory));
					 boost::shared_ptr<DsegMatcher> dsegMatcher(new DsegMatcher(configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE*SEGMENT_NOISE_FACTOR));
					 boost::shared_ptr<DataManager_ImageSeg_Test> dmSeg(new DataManager_ImageSeg_Test(hdsegDetector, dsegMatcher, assGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

					 dmSeg->linkToParentSensorSpec(senPtr11);
					 dmSeg->linkToParentMapManager(mmSeg);
					 dmSeg->setObservationFactory(obsFact);
			#endif
			#if SEGMENT_BASED != 1
					 if (configEstimation.MULTIVIEW_DESCRIPTOR)
							pointDescFactory.reset(new DescriptorImagePointMultiViewFactory(configEstimation.DESC_SIZE, configEstimation.DESC_SCALE_STEP, jmath::degToRad(configEstimation.DESC_ANGLE_STEP), (DescriptorImagePointMultiView::PredictionType)configEstimation.DESC_PREDICTION_TYPE));
					 else
							pointDescFactory.reset(new DescriptorImagePointFirstViewFactory(configEstimation.DESC_SIZE));

					 boost::shared_ptr<ImagePointHarrisDetector> harrisDetector(new ImagePointHarrisDetector(configEstimation.HARRIS_CONV_SIZE, configEstimation.HARRIS_TH, configEstimation.HARRIS_EDDGE, configEstimation.PATCH_SIZE, configEstimation.PIX_NOISE, pointDescFactory));
					 boost::shared_ptr<ImagePointZnccMatcher> znccMatcher(new ImagePointZnccMatcher(configEstimation.MIN_SCORE, configEstimation.PARTIAL_POSITION, configEstimation.PATCH_SIZE, configEstimation.MAX_SEARCH_SIZE, configEstimation.RANSAC_LOW_INNOV, configEstimation.MATCH_TH, configEstimation.MAHALANOBIS_TH, configEstimation.RELEVANCE_TH, configEstimation.PIX_NOISE));

					 boost::shared_ptr<DataManager_ImagePoint_Ransac> dmPt11(new DataManager_ImagePoint_Ransac(harrisDetector, znccMatcher, asGrid, configEstimation.N_UPDATES_TOTAL, configEstimation.N_UPDATES_RANSAC, ransac_ntries, configEstimation.N_INIT, configEstimation.N_RECOMP_GAINS));

					 dmPt11->linkToParentSensorSpec(senPtr11);
					 dmPt11->linkToParentMapManager(mmPoint);
					 dmPt11->setObservationFactory(obsFact);
				#endif

			#ifdef HAVE_VIAM
			viam_hwcrop_t crop;
			switch (configSetup.CAMERA_TYPE)
			{
				case 0: crop = VIAM_HW_FIXED; break;
				case 1: crop = VIAM_HW_CROP; break;
				default: crop = VIAM_HW_FIXED; break;
			}
			hardware::hardware_sensor_firewire_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, 5+50,
				configSetup.CAMERA_DEVICE, cv::Size(img_width,img_height), 0, 8, crop, floatOpts[fFreq], intOpts[iTrigger],
				floatOpts[fShutter], mode, strOpts[sDataPath]));
			hardSen11->setTimingInfos(1.0/hardSen11->getFreq(), 1.0/hardSen11->getFreq());
			senPtr11->setHardwareSensor(hardSen11);
			senPtr11->setIntegrationPolicy(false);
			senPtr11->setUseForInit(false);
			senPtr11->setNeedInit(false);
			//hardSen11->start();
			#else
			if (intOpts[iReplay] & 1)
			{
				hardware::hardware_sensorext_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(rawdata_condition, cv::Size(img_width,img_height),strOpts[sDataPath]));
				senPtr11->setHardwareSensor(hardSen11);
				senPtr11->setIntegrationPolicy(false);
				senPtr11->setUseForInit(false);
				senPtr11->setNeedInit(false);
				//hardSen11->start();
			}
			#endif
		}
	} // if (intOpts[iCamera])

	if (intOpts[iGps])
	{
		absloc_ptr_t senPtr13(new SensorAbsloc(robPtr1, MapObject::UNFILTERED, false));
		senPtr13->setId();
		senPtr13->linkToParentRobot(robPtr1);
		hardware::hardware_sensorprop_ptr_t hardGps;
		bool inits = true;
		switch (intOpts[iGps])
		{
			case 1:
				hardGps.reset(new hardware::HardwareSensorGpsGenom(rawdata_condition, 200, "mana-base", mode, strOpts[sDataPath]));
			case 2:
				hardGps.reset(new hardware::HardwareSensorGpsGenom(rawdata_condition, 200, "mana-base", mode, strOpts[sDataPath])); // TODO ask to ignore vel
			case 3:
				hardGps.reset(new hardware::HardwareSensorMocap(rawdata_condition, 200, mode, strOpts[sDataPath]));
				inits = false;
		}

		hardGps->setSyncConfig(configSetup.GPS_TIMESTAMP_CORRECTION);
		hardGps->setTimingInfos(1.0/20.0, 1.5/20.0);
		senPtr13->setHardwareSensor(hardGps);
		senPtr13->setIntegrationPolicy(true);
		senPtr13->setUseForInit(inits);
		senPtr13->setNeedInit(inits);
		senPtr13->setPose(configSetup.GPS_POSE[0], configSetup.GPS_POSE[1], configSetup.GPS_POSE[2],
											configSetup.GPS_POSE[3], configSetup.GPS_POSE[4], configSetup.GPS_POSE[5]); // x,y,z,roll,pitch,yaw
		//hardGps->start();
	}
	
	if (intOpts[iReplay] == 1)
		sensorManager.reset(new SensorManagerReplay(mapPtr));
	else
		sensorManager.reset(new SensorManagerOneAndOne(mapPtr));
	
	//--- force a first display with empty slam to ensure that all windows are loaded
// std::cout << "SLAM: forcing first initialization display" << std::endl;
	#ifdef HAVE_MODULE_QDISPLAY
	if (intOpts[iDispQt])
	{
		viewerQt = PTR_CAST<display::ViewerQt*> (worldPtr->getDisplayViewer(display::ViewerQt::id()));
		viewerQt->bufferize(worldPtr);
		
		// initializing stuff for controlling run/pause from viewer
		boost::unique_lock<boost::mutex> runStatus_lock(viewerQt->runStatus.mutex);
		viewerQt->runStatus.pause = intOpts[iPause];
		viewerQt->runStatus.render_all = intOpts[iRenderAll];
		runStatus_lock.unlock();
	}
	#endif
	#ifdef HAVE_MODULE_GDHE
	if (intOpts[iDispGdhe])
	{
		viewerGdhe = PTR_CAST<display::ViewerGdhe*> (worldPtr->getDisplayViewer(display::ViewerGdhe::id()));
		viewerGdhe->bufferize(worldPtr);
	}
	#endif

// std::cout << "SLAM: starting slam" << std::endl;

	// Show empty map
//	std::cout << *mapPtr << std::endl;

	//worldPtr->display_mutex.unlock();

	switch (intOpts[iExport])
	{
		case 1: exporter.reset(new ExporterSocket(robPtr1, 30000)); break;
		case 2: exporter.reset(new ExporterPoster(robPtr1)); break;
	}

} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } } // demo_slam_init




void demo_slam_main(world_ptr_t *world)
{ try {

	robot_ptr_t robotPtr;
		
	// wait for display to be ready if enabled
	if (intOpts[iDispQt] || intOpts[iDispGdhe])
	{
		boost::unique_lock<boost::mutex> display_lock(worldPtr->display_mutex);
		worldPtr->display_rendered = false;
		display_lock.unlock();
		worldPtr->display_condition.notify_all();
// std::cout << "SLAM: now waiting for this display to finish" << std::endl;
		display_lock.lock();
		while(!worldPtr->display_rendered) worldPtr->display_condition.wait(display_lock);
		display_lock.unlock();
	}

	// start hardware sensors that need long init
	map_ptr_t mapPtr = (*world)->mapList().front();
	for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
		robIter != mapPtr->robotList().end(); ++robIter)
	{
		if ((*robIter)->hardwareEstimatorPtr) (*robIter)->hardwareEstimatorPtr->start();
		for (RobotAbstract::SensorList::iterator senIter = (*robIter)->sensorList().begin();
			senIter != (*robIter)->sensorList().end(); ++senIter)
		{
			if ((*senIter)->getNeedInit())
				(*senIter)->start();
		}
		robotPtr = *robIter;
	}

	// wait for their init
	std::cout << "Sensors are calibrating..." << std::flush;
	if ((intOpts[iRobot] == 1 || intOpts[iGps] == 1 || intOpts[iGps] == 2) && !(intOpts[iReplay] & 1)) sleep(2);
	std::cout << " done." << std::endl;
					
	// set the start date
	double start_date = kernel::Clock::getTime();
	if (!(intOpts[iReplay] & 1) && intOpts[iDump]) {
		std::fstream f((strOpts[sDataPath] + std::string("/sdate.log")).c_str(), std::ios_base::out);
		f << std::setprecision(19) << start_date << std::endl;
		f.close();
	}
	else if (intOpts[iReplay] & 1) {
		std::fstream f((strOpts[sDataPath] + std::string("/sdate.log")).c_str(), std::ios_base::in);
		if (!f.is_open()) std::cout << "Missing sdate.log file" << std::endl;
		f >> start_date;
		f.close();
	}
	std::cout << "slam start date: " << std::setprecision(16) << start_date << std::endl;
	sensorManager->setStartDate(start_date);
	
	// start other hardware sensors
	for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin();
		robIter != mapPtr->robotList().end(); ++robIter)
	{
		for (RobotAbstract::SensorList::iterator senIter = (*robIter)->sensorList().begin();
			senIter != (*robIter)->sensorList().end(); ++senIter)
		{
			if (!(*senIter)->getNeedInit())
				(*senIter)->start();
		}
	}
		
		
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

	for (; (*world)->t <= N_FRAMES;)
	{
		if ((*world)->exit()) break;
		
		bool had_data = false;
		chrono.reset();

		SensorManagerAbstract::ProcessInfo pinfo = sensorManager->getNextDataToUse();
		bool no_more_data = pinfo.no_more_data;
		
		if (pinfo.sen)
		{
			had_data = true;
			if (intOpts[iReplay] == 2)
				pinfo.sen->process_fake(pinfo.id); // just to release data
			else
			{
				double newt = pinfo.sen->getRawTimestamp(pinfo.id);
				
				JFR_DEBUG("************** FRAME : " << (*world)->t << " (" << std::setprecision(16) << newt << std::setprecision(6) << ") sensor " << pinfo.sen->id());
				
				robot_ptr_t robPtr = pinfo.sen->robotPtr();
//std::cout << "Frame " << (*world)->t << " using sen " << pinfo.sen->id() << " at time " << std::setprecision(16) << newt << std::endl;
				robPtr->move(newt);
				
				jblas::vec euler = quaternion::q2e(ublas::subrange(robPtr->state.x(), 3, 7));
				JFR_DEBUG("Robot " << robPtr->id() << " state after move " << robPtr->state.x() << " ; euler " << euler);
				JFR_DEBUG("Robot state stdev after move " << stdevFromCov(robPtr->state.P()));
				robot_prediction = robPtr->state.x();
				
				pinfo.sen->process(pinfo.id);
				
				euler = quaternion::q2e(ublas::subrange(robPtr->state.x(), 3, 7));
				JFR_DEBUG("Robot state after corrections of sensor " << pinfo.sen->id() << " : " << robPtr->state.x() << " ; euler " << euler);
				JFR_DEBUG("Robot state stdev after corrections " << stdevFromCov(robPtr->state.P()));
				average_robot_innovation += ublas::norm_2(robPtr->state.x() - robot_prediction);
				n_innovation++;
				
				if (exporter) exporter->exportCurrentState();
#ifdef GENOM // export genom
				jblas::vec euler_x(3);
				jblas::sym_mat euler_P(3,3);
				quaternion::q2e(ublas::subrange(robotPtr->state.x(), 3, 7), ublas::subrange(robotPtr->state.P(), 3,7, 3,7), euler_x, euler_P);
				jblas::vec stateX(6);
				ublas::subrange(stateX,0,3) = ublas::subrange(robotPtr->state.x(),0,3)+robotPtr->origin_sensors-robotPtr->origin_export;
				ublas::subrange(stateX,3,6) = euler_x;
				jblas::vec stateP(6);
				for(int i = 0; i < 3; ++i) stateP(i) = robotPtr->state.P(i,i);
				for(int i = 0; i < 3; ++i) stateP(3+i) = euler_P(i,i);
				updatePoster(newt, (*world)->t, stateX, stateP);
#endif
			}
		}
		

		// wait that display has finished if render all
		if (had_data)
		{
			// get render all status
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

			// if render all, wait display has finished
			if ((intOpts[iDispQt] || intOpts[iDispGdhe]) && renderAll)
			{
				boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
				while(!(*world)->display_rendered && !(*world)->exit()) (*world)->display_condition.wait(display_lock);
				display_lock.unlock();
			}
		}

		
		// asking for display if display has finished
		unsigned processed_t = (had_data ? (*world)->t : (*world)->t-1);
		if ((*world)->display_t+1 < processed_t+1)
		{
			boost::unique_lock<boost::mutex> display_lock((*world)->display_mutex);
			if ((*world)->display_rendered)
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
				display_lock.unlock();
				(*world)->display_condition.notify_all();
			} else
			display_lock.unlock();
		}
		
		if (no_more_data) break;

		if (!had_data)
		{
			rawdata_condition.wait(boost::lambda::_1 != 0);
			rawdata_condition.set(0);
		}
		
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

		if (had_data)
		{
			(*world)->t++;
			if (dataLogger) dataLogger->log();
		}
	} // temporal loop


	average_robot_innovation /= n_innovation;
	std::cout << "average_robot_innovation " << average_robot_innovation << std::endl;

	if (exporter) exporter->stop();
	(*world)->slam_blocked(true);
//	std::cout << "\nFINISHED ! Press a key to terminate." << std::endl;
//	getchar();

} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } } // demo_slam_main


/** ############################################################################
 * #############################################################################
 * Display function
 * ###########################################################################*/

void demo_slam_display(world_ptr_t *world)
{ try {
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
	
} catch (kernel::Exception &e) { std::cout << e.what(); throw e; } }


/** ############################################################################
 * #############################################################################
 * Exit function
 * ###########################################################################*/

void demo_slam_exit(world_ptr_t *world, boost::thread *thread_main) {
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


void demo_slam_run() {

	// to start with qt display
	if (intOpts[iDispQt]) // at least 2d
	{
		#ifdef HAVE_MODULE_QDISPLAY
		qdisplay::QtAppStart((qdisplay::FUNC)&demo_slam_display,display_priority,(qdisplay::FUNC)&demo_slam_main,slam_priority,display_period,&worldPtr,(qdisplay::EXIT_FUNC)&demo_slam_exit);
		#else
		std::cout << "Please install qdisplay module if you want 2D display" << std::endl;
		#endif
	} else
	if (intOpts[iDispGdhe]) // only 3d
	{
		#ifdef HAVE_MODULE_GDHE
		kernel::setCurrentThreadPriority(display_priority);
		boost::thread *thread_disp = new boost::thread(boost::bind(demo_slam_display,&worldPtr));
		kernel::setCurrentThreadPriority(slam_priority);
		demo_slam_main(&worldPtr);
		delete thread_disp;
		#else
		std::cout << "Please install gdhe module if you want 3D display" << std::endl;
		#endif
	} else // none
	{
		kernel::setCurrentThreadPriority(slam_priority);
		demo_slam_main(&worldPtr);
	}

	JFR_DEBUG("Terminated");
}



/** ############################################################################
 * #############################################################################
 * main function
 * ###########################################################################*/

#ifndef GENOM

/**
	* Program options:
	* --disp-2d=0/1
	* --disp-3d=0/1
	* --render-all=0/1 (needs --replay 1)
	* --replay=0/1/2/3 (off/on/off no slam/on true time) (needs --data-path)
	* --dump=0/1  (needs --data-path)
	* --rand-seed=0/1/n, 0=generate new one, 1=in replay use the saved one, n=use seed n
	* --pause=0/n 0=don't, n=pause for frames>n (needs --replay 1)
	* --log=0/1/filename -> log result in text file
	* --export=0/1/2 -> Off/socket/poster
	* --verbose=0/1/2/3/4/5 -> Off/Trace/Warning/Debug/VerboseDebug/VeryVerboseDebug
	* --data-path=/mnt/ram/rtslam
	* --config-setup=data/setup.cfg
	* --config-estimation=data/estimation.cfg
	* --help
	* --usage
	* --robot 0=constant vel, 1=inertial
	* --map 0=odometry, 1=global, 2=local/multimap
	* --trigger 0=internal, 1=external mode 1, 2=external mode 0, 3=external mode 14 (PointGrey (Flea) only)
	* --simu 0 or <environment id>*10+<trajectory id> (
	* --camera=0/1/2/3 -> Disable / Mono / Stereo / Bicam
	* --freq camera frequency in double Hz (with trigger==0/1)
	* --shutter shutter time in double seconds (0=auto); for trigger modes 0,2,3 the value is relative between 0 and 1
	* --gps=0/1/2/3 -> Off / Pos / Pos+Vel / Pos+Ori(mocap)
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
{ try {
	intOpts[iVerbose] = 5;
	intOpts[iMap] = 1;
	intOpts[iCamera] = 1;
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
	
	demo_slam_init();
	demo_slam_run();
	
} catch (kernel::Exception &e) { std::cout << e.what();  throw e; } }

#endif

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
	KeyValueFile_getItem(GPS_POSE);
	KeyValueFile_getItem(ROBOT_POSE);
	
	KeyValueFile_getItem(CAMERA_TYPE);
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
	
	KeyValueFile_getItem(UNCERT_HEADING);
	KeyValueFile_getItem(UNCERT_ATTITUDE);
	
	KeyValueFile_getItem(IMU_TIMESTAMP_CORRECTION);
	KeyValueFile_getItem(GPS_TIMESTAMP_CORRECTION);
	
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
	KeyValueFile_setItem(GPS_POSE);
	KeyValueFile_setItem(ROBOT_POSE);
	
	KeyValueFile_setItem(CAMERA_TYPE);
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
	
	KeyValueFile_setItem(UNCERT_HEADING);
	KeyValueFile_setItem(UNCERT_ATTITUDE);
	
	KeyValueFile_setItem(IMU_TIMESTAMP_CORRECTION);
	KeyValueFile_setItem(GPS_TIMESTAMP_CORRECTION);
	
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
	KeyValueFile_getItem(MULTIVIEW_DESCRIPTOR);
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
	KeyValueFile_setItem(MULTIVIEW_DESCRIPTOR);
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
