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

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "kernel/timingTools.hpp"
#include "jmath/random.hpp"
#include "jmath/matlab.hpp"
#include "jmath/ublasExtra.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/robotOdometry.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/robotInertial.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationMakers.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/dataManagerActiveSearch.hpp"

//#include "rtslam/hardwareSensorCameraFirewire.hpp"

#include "rtslam/display_qt.hpp"
//#include "image/Image.hpp"

//#include <map>

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

void demo_slam01_main(world_ptr_t *world) {

	const int MAPSIZE = 200;
	const int NFRAME = 5000;

	const double FRAMERATE = 60;

	int imgWidth = 640, imgHeight = 480;
	boost::shared_ptr<ActiveSearchGrid> asGrid(new ActiveSearchGrid(imgWidth, imgHeight, 4, 4, 16, 5));

	double _d[3] = { -0.27965, 0.20059, -0.14215 };
	vec d = createVector<3> (_d);
	double _k[4] = { 551.379, 365.793, 1079.2, 1076.73 };
	vec k = createVector<4> (_k);

	boost::shared_ptr<ObservationFactory> obsFact(new ObservationFactory());
	obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker()));
	obsFact->addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker()));


	// ---------------------------------------------------------------------------
	// --- INIT ------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	// INIT : 1 map and map-manager, 2 robs, 3 sens and data-manager.
	world_ptr_t worldPtr = *world;
	worldPtr->display_mutex.lock();


	// 1. Create maps.
	map_ptr_t mapPtr(new MapAbstract(MAPSIZE));
	worldPtr->addMap(mapPtr);
	mapPtr->clear();
	// 1b. Create map manager.
	boost::shared_ptr<MapManager<LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> > mmPoint(new MapManager<
	    LandmarkAnchoredHomogeneousPoint, LandmarkEuclideanPoint> ());
	mmPoint->linkToParentMap(mapPtr);
	mmPoint->setObservationFactory(obsFact);


	// 2. Create robots.
	robconstvel_ptr_t robPtr1(new RobotConstantVelocity(mapPtr));
	robPtr1->setId();
	robPtr1->linkToParentMap(mapPtr);
	vec v(robPtr1->mySize());
	fillVector(v, 0.0);
	robPtr1->state.x(v);
	robPtr1->pose.x(quaternion::originFrame());
	robPtr1->dt_or_dx = 1 / FRAMERATE;
	v.resize(robPtr1->mySize_perturbation());
	fillVector(v, 1.0);
	robPtr1->perturbation.clear();
	robPtr1->perturbation.set_std_continuous(v);
	robPtr1->constantPerturbation = false;

	// 3. Create sensors.
	pinhole_ptr_t senPtr11(new SensorPinHole(robPtr1, MapObject::UNFILTERED));
	senPtr11->setId();
	senPtr11->linkToParentRobot(robPtr1);
	senPtr11->state.clear();
	senPtr11->pose.x(quaternion::originFrame());
	senPtr11->params.setImgSize(imgWidth, imgHeight);
	senPtr11->params.setIntrinsicCalibration(k, d, d.size());
	// 3b. Create data manager.
	boost::shared_ptr<DataManagerActiveSearch<RawImage, SensorPinHole> > dmPt11(new DataManagerActiveSearch<RawImage,
	    SensorPinHole> ());
	dmPt11->linkToParentSensorSpec(senPtr11);
	dmPt11->linkToParentMapManager(mmPoint);
	dmPt11->setActiveSearchGrid(asGrid);
	/* TODO-NMSD: this should not be in sensor ... */
	senPtr11->params.setMiscellaneous(1.0, 0.1, DataManagerActiveSearch<RawImage, SensorPinHole>::patchMatchSize);


	//viam_hwmode_t hwmode = { VIAM_HWSZ_640x480, VIAM_HWFMT_MONO8, VIAM_HW_FIXED, VIAM_HWFPS_60, VIAM_HWTRIGGER_INTERNAL };
	// UNCOMMENT THESE TWO LINES ENABLE FIREWIRE CAMERA OPERATION
	//hardware_sensor_ptr_t hardSen11(new HardwareSensorCameraFirewire("0x00b09d01006fb38f", hwmode));
	//senPtr11->setHardwareSensor(hardSen11);
	cout << "d: " << senPtr11->params.distortion << "\nc: " << senPtr11->params.correction << endl;


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
	kernel::Chrono total_chrono;
	kernel::Chrono mutex_chrono;
	int dt, max_dt = 0;
	for (int t = 1; t <= NFRAME;) {
		bool had_data = false;

		worldPtr->display_mutex.lock();
		// cout << "\n************************************************** " << endl;
		// cout << "\n                 FRAME : " << t << " (blocked "
		//      << mutex_chrono.elapsedMicrosecond() << " us)" << endl;
		chrono.reset();


		// foreach robot
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++) {
			robot_ptr_t robPtr = *robIter;


			// cout << "\n================================================== " << endl;
			// cout << *robPtr << endl;

			// foreach sensor
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter
			    != robPtr->sensorList().end(); senIter++) {
				sensor_ptr_t senPtr = *senIter;
				cout << "\n________________________________________________ " << endl;
				cout << *senPtr << endl;


				// get raw-data
				if (senPtr->acquireRaw() < 0)
					continue;
				//std::cout << chronototal.elapsed() << " has acquired" << std::endl;

				// move the filter time to the data raw.
				vec u(robPtr->mySize_control()); // TODO put some real values in u.
				fillVector(u, 0.0);
				robPtr->move(u, senPtr->getRaw()->timestamp);


				// foreach dataManager
				for (SensorAbstract::DataManagerList::iterator dmaIter = senPtr->dataManagerList().begin(); dmaIter
				    != senPtr->dataManagerList().end(); dmaIter++) {
					data_manager_ptr_t dmaPtr = *dmaIter;
					dmaPtr->process(senPtr->getRaw());
				} // foreach dataManager

				senPtr->releaseRaw();
				had_data = true;

			} // for each sensor
		} // for each robot

		cout << "total lmks: " << (*(mapPtr->mapManagerList().begin()))->landmarkList().size() << endl;

		dt = chrono.elapsedMicrosecond();
		if (dt > max_dt)
			max_dt = dt;
		if (had_data) {
			t++;
			vec7 F = robPtr1->pose.x();
			vec3 p = subrange(F, 0, 3);
			vec4 q = subrange(F, 3, 7);
			vec3 e = quaternion::q2e(q);
			cout << "dt: " << (int) (1000 * robPtr1->dt_or_dx) << "ms. " << "p: " << 100 * p << " cm." << "e: " << (180.0
			    / 3.14) * e << " deg." << endl;
		}
		worldPtr->display_mutex.unlock();
		mutex_chrono.reset();

	} // temporal loop

	std::cout << "average time : " << total_chrono.elapsed() / NFRAME << " ms," << "max frame time " << max_dt
	    << std::endl;
	std::cout << "\nFINISHED !" << std::endl;

	sleep(60);
} // demo_slam01_main


void demo_slam01_display(world_ptr_t *world) {
	//(*world)->display_mutex.lock();
	qdisplay::qtMutexLock<kernel::FifoMutex>((*world)->display_mutex);
	display::ViewerQt *viewerQt = static_cast<display::ViewerQt*> ((*world)->getDisplayViewer(display::ViewerQt::id()));
	if (viewerQt == NULL) {
		viewerQt = new display::ViewerQt();
		(*world)->addDisplayViewer(viewerQt, display::ViewerQt::id());
	}
	viewerQt->bufferize(*world);
	(*world)->display_mutex.unlock();

	viewerQt->render();
}

void demo_slam01() {
	world_ptr_t worldPtr(new WorldAbstract());


	// to start with qt display
	const int slam_priority = 10; // needs to be started as root to be > 0
	const int display_priority = -10;
	const int display_period = 100; // ms
	qdisplay::QtAppStart((qdisplay::FUNC) & demo_slam01_display, slam_priority, (qdisplay::FUNC) & demo_slam01_main,
	                     display_priority, display_period, &worldPtr);
	// to start without display
	//demo_slam01_main(&worldPtr);

	JFR_DEBUG("Terminated");
}

int main() {
	demo_slam01();
}
