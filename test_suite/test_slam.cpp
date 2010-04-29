/**
 * \file test_slam.cpp
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

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "jmath/random.hpp"
#include "jmath/matlab.hpp"

#include <iostream>
#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/robotOdometry.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/robotInertial.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/observationPinHolePoint.hpp"
#include "rtslam/featureAbstract.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;

jafar::kernel::IdFactory RobotAbstract::robotIds = jafar::kernel::IdFactory();
jafar::kernel::IdFactory SensorAbstract::sensorIds = jafar::kernel::IdFactory();
jafar::kernel::IdFactory LandmarkAbstract::landmarkIds = jafar::kernel::IdFactory();

void test_slam01() {
	ActiveSearchGrid acGrid(640, 480, 8, 8, 10);
	// INIT : 1 map, 2 robs, 3 sens
	map_ptr_t mapPtr(new MapAbstract(100));
	robot_ptr_t robPtr1(new RobotConstantVelocity(mapPtr));
	robPtr1->linkToParentMap(mapPtr);
	sensor_ptr_t senPtr11 (new SensorPinHole(robPtr1, MapObject::UNFILTERED));
	senPtr11->linkToParentRobot(robPtr1);
	sensor_ptr_t senPtr12 (new SensorPinHole(robPtr1, MapObject::FILTERED));
	senPtr12->linkToParentRobot(robPtr1);
	robot_ptr_t robPtr2(new RobotOdometry(mapPtr));
	robPtr2->linkToParentMap(mapPtr);
	sensor_ptr_t senPtr21 (new SensorPinHole(robPtr2, MapObject::UNFILTERED));
	senPtr21->linkToParentRobot(robPtr2);

	sensor_ptr_t senPtrCopy;
	senPtrCopy = senPtr11;


	if (senPtr11 == senPtrCopy){
		cout << "EQ" << endl;
	}else{
		cout << "NEQ" << endl;
	}



	// INIT : complete observations set
	// loop all sensors
	// loop all lmks
	// create sen--lmk observation
	// Temporal loop
	for (int t = 1; t <= 100; t++) {

		// foreach robot
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++)
		{
			robot_ptr_t robPtr = *robIter;
			vec u(robPtr->size_control()); // TODO put some real values in u.
			robPtr->set_control(u);
			robPtr->move();
			// foreach sensor
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++)
			{
				sensor_ptr_t senPtr = *senIter;
				// get raw-data
				senPtr->acquireRaw() ; // todo define-it in sensorPinHole

				// 1. Observe known landmarks
				// foreach observation
				for (SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter != senPtr->observationList().end(); obsIter++)
				{
					observation_ptr_t obsPtr = *obsIter;
					obsPtr->clearEvents();
					// 1a. project
					obsPtr->project();
					// 1b. check visibility
					obsPtr->predictVisibility();
					if (obsPtr->isVisible()){
						acGrid.addPixel(obsPtr->expectation.x());
						obsPtr->counters.nSearch++;
						// 1c. predict appearance
						obsPtr->predictAppearance();

						// 1d. search appearence in raw
						obsPtr->matchFeature(senPtr->getRaw()) ;

						// 1e. if feature is find
						if (obsPtr->getMatchScore()>80) {
							obsPtr->counters.nMatch++;
							obsPtr->events.matched = true;
							obsPtr->computeInnovation() ;
							if (obsPtr->compatibilityTest(3.0)) {
								obsPtr->counters.nInlier++;
								obsPtr->update() ;
								obsPtr->events.updated = true;
							} // obsPtr->compatibilityTest(3.0)
						} // obsPtr->getScoreMatchInPercent()>80
					} // obsPtr->isVisible()
				} // foreach observation




				// 2 init new landmarks
				if (mapPtr->unusedStates(LandmarkAnchoredHomogeneousPoint::size())) {
					ROI roi;
					if (acGrid.getROI(roi)){
						feature_ptr_t featPtr;
						if (ObservationPinHolePoint::detectInRoi(senPtr->getRaw(), roi, featPtr)){

							// create lmk object
							landmark_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr));
							lmkPtr->linkToParentMap(mapPtr);

							// create all obs objects
							// todo make lmk creation dynamic with factories or switch or other.
							observation_ptr_t obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint(senPtr,lmkPtr));
							obsPtr->linkToParentSensor(senPtr);
							obsPtr->linkToParentLandmark(lmkPtr);

							// fill data for this obs
							obsPtr->setup(featPtr, ObservationPinHoleAnchoredHomogeneousPoint::getPrior());

							// fill data for the landmark
							obsPtr->backProject();

							lmkPtr->createDescriptor(featPtr->appearancePtr, senPtr->globalPose());
//							obsPtr->createDescriptor(featPtr->appearancePtr, senPtr->globalPose());

							// Complete with all other obs
							mapPtr->completeObservationsInGraph(senPtr, lmkPtr);

						}
					}
				}

			}
		}


	}


}


BOOST_AUTO_TEST_CASE( test_slam )
{
	test_slam01();
}
