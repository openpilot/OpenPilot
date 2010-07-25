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
#include <time.h>
#include <map>
#include <getopt.h>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "kernel/timingTools.hpp"
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

typedef ImagePointObservationMaker<ObservationPinHoleEuclideanPoint, SensorPinHole, LandmarkEuclideanPoint,
    SensorAbstract::PINHOLE, LandmarkAbstract::PNT_EUC> PinholeEucpObservationMaker;
typedef ImagePointObservationMaker<ObservationPinHoleAnchoredHomogeneousPoint, SensorPinHole,
    LandmarkAnchoredHomogeneousPoint, SensorAbstract::PINHOLE, LandmarkAbstract::PNT_AH> PinholeAhpObservationMaker;

typedef DataManagerOnePointRansac<RawImage, SensorPinHole, FeatureImagePoint, image::ConvexRoi, ActiveSearchGrid, ImagePointHarrisDetector, ImagePointZnccMatcher> DataManager_ImagePoint_Ransac;
typedef DataManagerOnePointRansac<simu::RawSimu, SensorPinHole, simu::FeatureSimu, image::ConvexRoi, ActiveSearchGrid, simu::DetectorSimu<image::ConvexRoi>, simu::MatcherSimu<image::ConvexRoi> > DataManager_ImagePoint_Ransac_Simu;

///##############################################

#define RANSAC 1

int mode = 0;
time_t rseed;

///##############################################

enum { iDispQt = 0, iDispGdhe, iRenderAll, iReplay, iDump, iRandSeed, iPause, iLog, iVerbose, iRobot, iTrigger, iSimu, nIntOpts };
int intOpts[nIntOpts] = {0};

enum { fFreq = 0, fShutter, nFloatOpts };
double floatOpts[nFloatOpts] = {0.0};

enum { sSlamConfig = 0, sDataPath, nStrOpts };
std::string strOpts[nStrOpts];


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

// time
const unsigned N_FRAMES = 500000;

// map
const unsigned MAP_SIZE = 313;

// constant velocity robot uncertainties and perturbations
const double UNCERT_VLIN = .1; // m/s
const double UNCERT_VANG = .1; // rad/s
const double PERT_VLIN = 1; // m/s per sqrt(s)
const double PERT_VANG = 3; // rad/s per sqrt(s)

// inertial robot initial uncertainties and perturbations
//if (intOpts[iRobot] == 1) // == robot inertial
const double UNCERT_GRAVITY = 10.0; // m/s^2
const double UNCERT_ABIAS = 0.05*17.0;
const double UNCERT_WBIAS = 0.05*jmath::degToRad(300.0);
const double PERT_AERR = 0.2*0.001*sqrt(30.0/100.0); // m/s per sqrt(s), IMU acc error (MTI = 0.001*sqrt(40Hz/100Hz))
const double PERT_WERR = 0.2*jmath::degToRad(0.05)*sqrt(40.0/100.0); // rad per sqrt(s), IMU gyro error (MTI = 0.05*sqrt(30Hz/100Hz))
const double PERT_RANWALKACC = 0; // m/s^2 per sqrt(s), IMU a_bias random walk
const double PERT_RANWALKGYRO = 0; // rad/s^2 per sqrt(s), IMU w_bias random walk

// pin-hole:
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 301.27013,   266.86136,   497.28243,   496.81116 };
const double DISTORTION[2] = { -0.23193,   0.11306 }; //{-0.27965, 0.20059, -0.14215}; //{-0.27572, 0.28827};
const double PIX_NOISE = .5;
const double PIX_NOISE_SIMUFACTOR = 0.0;

// lmk management
const double D_MIN = .5;
const double REPARAM_TH = 0.1;

// data manager: quick Harris detector
const unsigned HARRIS_CONV_SIZE = 5;
const double HARRIS_TH = 20.0;
const double HARRIS_EDDGE = 3.0;
const unsigned PATCH_DESC = 45;

// data manager: zncc matcher and one-point-ransac
const unsigned PATCH_SIZE = 15; // in pixels
const double MATCH_TH = 0.90;
const double MAHALANOBIS_TH = 3; // in n_sigmas
const unsigned N_UPDATES_TOTAL = 25;
const unsigned N_UPDATES_RANSAC = 20;
const unsigned N_INIT = 10;
const unsigned N_RECOMP_GAINS = 3;
const double RANSAC_LOW_INNOV = 1.0; // in pixels
#if RANSAC
const unsigned RANSAC_NTRIES = 6;
#else
const unsigned RANSAC_NTRIES = 0;
#endif
const double MIN_SCORE = 0.8;
const double PARTIAL_POSITION = 0.25;

// data manager: active search tesselation grid
const unsigned GRID_VCELLS = 3;
const unsigned GRID_HCELLS = 4;
const unsigned GRID_MARGIN = 11;
const unsigned GRID_SEPAR = 20;


///##############################################

void demo_slam01_main(world_ptr_t *world) {


	// pin-hole parameters in BOOST format
	vec intrinsic = createVector<4> (INTRINSIC);
	vec distortion = createVector<sizeof(DISTORTION)/sizeof(double)> (DISTORTION);

	boost::shared_ptr<ObservationFactory> obsFact(new ObservationFactory());
	obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker(PATCH_SIZE, D_MIN)));
	obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker(PATCH_SIZE, D_MIN, REPARAM_TH)));


	// ---------------------------------------------------------------------------
	// --- INIT ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : 1 map and map-manager, 2 robs, 3 sens and data-manager.
	world_ptr_t worldPtr = *world;
	worldPtr->display_mutex.lock();


	// 1. Create maps.
	map_ptr_t mapPtr(new MapAbstract(MAP_SIZE));
	worldPtr->addMap(mapPtr);
	// 1b. Create map manager.
	boost::shared_ptr<MapManager<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> > mmPoint(new MapManager<
	    LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> ());
	mmPoint->linkToParentMap(mapPtr);

	boost::shared_ptr<simu::AdhocSimulator> simulator;
	if (intOpts[iSimu] == 1)
	{
		simulator.reset(new simu::AdhocSimulator());
		jblas::vec3 pose;
		for(int i = -6; i <= 6; ++i)
			for(int j = -3; j <= 7; ++j)
			{
				pose(0) = i*1.0; pose(1) = j*1.0; pose(2) = 0.0;
				simu::Landmark *lmk = new simu::Landmark(LandmarkAbstract::POINT, pose);
				simulator->addLandmark(lmk);
			}
	}


	// 2. Create robots.
	robot_ptr_t robPtr1;
	if (intOpts[iRobot] == 0)
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
	} else
	if (intOpts[iRobot] == 1)
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
		if (intOpts[iSimu] == 1)
		{
			// TODO
			//boost::shared_ptr<hardware::HardwareEstimatorSimu> hardEst1_(new hardware::HardwareEstimatorSimu(
			//	"/dev/ttyS0", floatOpts[fFreq], floatOpts[fShutter], 1024, mode, strOpts[sDataPath]));
			//hardEst1 = hardEst1_;
		} else
		{
			boost::shared_ptr<hardware::HardwareEstimatorMti> hardEst1_(new hardware::HardwareEstimatorMti(
				"/dev/ttyS0", floatOpts[fFreq], floatOpts[fShutter], 1024, mode, strOpts[sDataPath]));
			if (intOpts[iTrigger]) floatOpts[fFreq] = hardEst1_->getFreq();
			hardEst1 = hardEst1_;
		}
		robPtr1_->setHardwareEstimator(hardEst1);

		robPtr1 = robPtr1_;
	}

	robPtr1->setId();
	robPtr1->linkToParentMap(mapPtr);
	robPtr1->pose.x(quaternion::originFrame());

	if (intOpts[iSimu] == 1)
	{
		simu::Robot *rob = new simu::Robot(robPtr1->id(), 6);
		double VEL = 0.5;
		rob->addWaypoint(0,0,0, 0,0,0, 0,0,0, 0,0,0);
		rob->addWaypoint(1,0,0, 0,0,0, VEL,0,0, 0,0,0);
		rob->addWaypoint(3,2,0, 0,0,0, 0,VEL,0, 0,0,0);
		rob->addWaypoint(1,4,0, 0,0,0, -VEL,0,0, 0,0,0);
		rob->addWaypoint(-1,4,0, 0,0,0, -VEL,0,0, 0,0,0);
		rob->addWaypoint(-3,2,0, 0,0,0, 0,-VEL,0, 0,0,0);
		rob->addWaypoint(-1,0,0, 0,0,0, VEL,0,0, 0,0,0);
		rob->addWaypoint(0,0,0, 0,0,0, 0,0,0, 0,0,0);
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
	senPtr11->params.setImgSize(IMG_WIDTH, IMG_HEIGHT);
	senPtr11->params.setIntrinsicCalibration(intrinsic, distortion, distortion.size());
	senPtr11->params.setMiscellaneous(1.0, 0.1);

	if (intOpts[iSimu] == 1)
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
	boost::shared_ptr<ActiveSearchGrid> asGrid(new ActiveSearchGrid(IMG_WIDTH, IMG_HEIGHT, GRID_HCELLS, GRID_VCELLS, GRID_MARGIN, GRID_SEPAR));
	
	if (intOpts[iSimu] == 1)
	{
		boost::shared_ptr<simu::DetectorSimu<image::ConvexRoi> > detector(new simu::DetectorSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, PATCH_DESC, PIX_NOISE, PIX_NOISE*PIX_NOISE_SIMUFACTOR));
		boost::shared_ptr<simu::MatcherSimu<image::ConvexRoi> > matcher(new simu::MatcherSimu<image::ConvexRoi>(LandmarkAbstract::POINT, 2, PATCH_SIZE, RANSAC_LOW_INNOV, MATCH_TH, MAHALANOBIS_TH, PIX_NOISE, PIX_NOISE*PIX_NOISE_SIMUFACTOR));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac_Simu> dmPt11(new DataManager_ImagePoint_Ransac_Simu(detector, matcher, asGrid, N_UPDATES_TOTAL, N_UPDATES_RANSAC, RANSAC_NTRIES, N_INIT, N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);
		
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorAdhocSimulator(floatOpts[fFreq], simulator, senPtr11->id(), robPtr1->id()));
		senPtr11->setHardwareSensor(hardSen11);
	} else
	{
		boost::shared_ptr<ImagePointHarrisDetector> harrisDetector(new ImagePointHarrisDetector(HARRIS_CONV_SIZE, HARRIS_TH, HARRIS_EDDGE, PATCH_DESC, PIX_NOISE));
	//	boost::shared_ptr<correl::Explorer<correl::Zncc> > znccMatcher(new correl::Explorer<correl::Zncc>());
		boost::shared_ptr<ImagePointZnccMatcher> znccMatcher(new ImagePointZnccMatcher(MIN_SCORE, PARTIAL_POSITION, PATCH_SIZE, RANSAC_LOW_INNOV, MATCH_TH, MAHALANOBIS_TH, PIX_NOISE));
		
		boost::shared_ptr<DataManager_ImagePoint_Ransac> dmPt11(new DataManager_ImagePoint_Ransac(harrisDetector, znccMatcher, asGrid, N_UPDATES_TOTAL, N_UPDATES_RANSAC, RANSAC_NTRIES, N_INIT, N_RECOMP_GAINS));

		dmPt11->linkToParentSensorSpec(senPtr11);
		dmPt11->linkToParentMapManager(mmPoint);
		dmPt11->setObservationFactory(obsFact);

		
		#ifdef HAVE_VIAM
		hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(
			"0x00b09d01006fb38f", cv::Size(IMG_WIDTH,IMG_HEIGHT), 0, 8, floatOpts[fFreq], intOpts[iTrigger], mode, strOpts[sDataPath]));
		senPtr11->setHardwareSensor(hardSen11);
		#else
		if (intOpts[iReplay])
		{
			hardware::hardware_sensor_ptr_t hardSen11(new hardware::HardwareSensorCameraFirewire(cv::Size(IMG_WIDTH,IMG_HEIGHT),strOpts[sDataPath]));
			senPtr11->setHardwareSensor(hardSen11);
		}
		#endif
	}
	

	// Show empty map
	cout << *mapPtr << endl;

	worldPtr->display_mutex.unlock();


	// ---------------------------------------------------------------------------
	// --- LOOP ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : complete observations set
	// loop all sensors
	// loop all lmks
	// create sen--lmk observation
	// Temporal loop

	kernel::Chrono chrono;
	double max_dt = 0;
	for (; (*world)->t <= N_FRAMES;)
	{
		bool had_data = false;

		worldPtr->display_mutex.lock();
		if (intOpts[iRenderAll] && worldPtr->display_rendered != (*world)->t)
			{ worldPtr->display_mutex.unlock(); boost::this_thread::yield(); continue; }
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
				if (senPtr->acquireRaw() < 0)
					{ boost::this_thread::yield(); continue; }
				else had_data=true;
// cout << "\n************************************************** " << endl;
cout << "                 FRAME : " << (*world)->t << endl;
//				cout << "Robot: " << *robPtr << endl;
//				cout << "Pert: " << robPtr->perturbation.P() << "\nPert Jac: " << robPtr->XNEW_pert << "\nState pert: " << robPtr->Q << endl;
//				cout << "Robot state: " << robPtr->state.x() << endl;

				// move the filter time to the data raw.
				robPtr->move(senPtr->getRaw()->timestamp);

				// foreach dataManager
				for (SensorAbstract::DataManagerList::iterator dmaIter = senPtr->dataManagerList().begin();
					dmaIter != senPtr->dataManagerList().end(); ++dmaIter)
				{
					data_manager_ptr_t dmaPtr = *dmaIter;
					dmaPtr->process(senPtr->getRaw());
				} // foreach dataManager

			} // for each sensor
		} // for each robot


		// NOW LOOP FOR STATE SPACE - ALL MM
		if (had_data)
		{
			(*world)->t++;

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
		} // if had_data

		int t = (*world)->t;
		worldPtr->display_mutex.unlock();
		if (intOpts[iPause] != 0 && t >= intOpts[iPause] && had_data) getchar(); // wait for key in replay mode
//std::cout << "one frame " << (*world)->t << " : " << mode << " " << had_data << std::endl;
	} // temporal loop

	std::cout << "\nFINISHED ! Press a key to terminate." << std::endl;
	getchar();
} // demo_slam01_main


void demo_slam01_display(world_ptr_t *world) {
	static unsigned prev_t = 0;
	kernel::Timer timer(display_period*1000);
	while(true)
	{
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

		if ((*world)->t != prev_t)
		{
			prev_t = (*world)->t;
			(*world)->display_rendered = (*world)->t;
			
			#ifdef HAVE_MODULE_QDISPLAY
			display::ViewerQt *viewerQt = NULL;
			if (intOpts[iDispQt]) viewerQt = PTR_CAST<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
			#endif
			#ifdef HAVE_MODULE_GDHE
			display::ViewerGdhe *viewerGdhe = NULL;
			if (intOpts[iDispGdhe]) viewerGdhe = PTR_CAST<display::ViewerGdhe*> ((*world)->getDisplayViewer(display::ViewerGdhe::id()));
			#endif
			
			#ifdef HAVE_MODULE_QDISPLAY
			if (intOpts[iDispQt]) viewerQt->bufferize(*world);
			#endif
			#ifdef HAVE_MODULE_GDHE
			if (intOpts[iDispGdhe]) viewerGdhe->bufferize(*world);
			#endif
			
			if (!intOpts[iRenderAll]) // strange: if we always unlock here, qt.dump takes much more time...
				(*world)->display_mutex.unlock();
				
			#ifdef HAVE_MODULE_QDISPLAY
			if (intOpts[iDispQt]) viewerQt->render();
			#endif
			#ifdef HAVE_MODULE_GDHE
			if (intOpts[iDispGdhe]) viewerGdhe->render();
			#endif
			
			if (intOpts[iReplay] && intOpts[iDump])
			{
				if (intOpts[iDispQt])
				{
					std::ostringstream oss; oss << strOpts[sDataPath] << "/rendered-2D_%d-" << std::setw(6) << std::setfill('0') << prev_t-1 << ".png";
					viewerQt->dump(oss.str());
				}
				if (intOpts[iDispGdhe])
				{
					std::ostringstream oss; oss << strOpts[sDataPath] << "/rendered-3D_" << std::setw(6) << std::setfill('0') << prev_t-1 << ".ppm";
					viewerGdhe->dump(oss.str());
				}
				if (intOpts[iRenderAll])
					(*world)->display_mutex.unlock();
			}
		} else
		{
			(*world)->display_mutex.unlock();
			boost::this_thread::yield();
		}
		
		if (intOpts[iDispQt]) break;
		              else timer.wait();
	}
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
			worldPtr->addDisplayViewer(viewerGdhe, display::ViewerGdhe::id());
		}
		#endif

		// to start with qt display
		if (intOpts[iDispQt]) // at least 2d
		{
			#ifdef HAVE_MODULE_QDISPLAY
			qdisplay::QtAppStart((qdisplay::FUNC)&demo_slam01_display,display_priority,(qdisplay::FUNC)&demo_slam01_main,slam_priority,display_period,&worldPtr);
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
		 * #--log=0/1 -> not implemented yet
		 * #--verbose=0/1/2 -> not implemented yet
		 * --data-path=/mnt/ram/rtslam
		 * #--slam-config=data/config1.xml -> not implemented yet
		 * --help
		 * --usage
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
					if (option_index < nIntOpts)
					{
						intOpts[option_index] = 1;
						if (optarg) intOpts[option_index] = atoi(optarg);
					} else
					if (option_index < nIntOpts+nFloatOpts)
					{
						if (optarg) floatOpts[option_index-nIntOpts] = atof(optarg);
					} else
					if (option_index < nIntOpts+nFloatOpts+nStrOpts)
					{
						if (optarg) strOpts[option_index-nIntOpts-nFloatOpts] = optarg;
					} else
					{
						std::cout << "Integer options:" << std::endl;
						for(int i = 0; i < nIntOpts; ++i)
							std::cout << "\t--" << long_options[i].name << std::endl;
						
						std::cout << "Float options:" << std::endl;
						for(int i = 0; i < nFloatOpts; ++i)
							std::cout << "\t--" << long_options[i+nIntOpts].name << std::endl;

						std::cout << "String options:" << std::endl;
						for(int i = 0; i < nStrOpts; ++i)
							std::cout << "\t--" << long_options[i+nIntOpts+nFloatOpts].name << std::endl;
						
						std::cout << "Breaking options:" << std::endl;
						for(int i = 0; i < 2; ++i)
							std::cout << "\t--" << long_options[i+nIntOpts+nFloatOpts+nStrOpts].name << std::endl;
						
						return 0;
					}
				} else
				{
					std::cerr << "Unknown option " << c << std::endl;
				}
			}
			
		if (intOpts[iReplay]) mode = 2; else
				if (intOpts[iDump]) mode = 1; else
					mode = 0;
			
			demo_slam01();
		}
