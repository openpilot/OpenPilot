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


// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"
#include "kernel/timingTools.hpp"
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
#include "rtslam/observationFactory.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"

#include "rtslam/display_qt.hpp"
//#include "image/Image.hpp"

//#include <map>

using namespace jblas;
using namespace jafar;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;


void test_slam01_main(world_ptr_t *world) {
	ActiveSearchGrid asGrid(640, 480, 8, 8, 22, 3);
	int imgWidth = 640, imgHeight = 480;
	vec4 k;
	vec d(0), c(0);
	k(0) = 320; k(1) = 320; k(2) = 320; k(3) = 320;
	int patchMatchSize = 11;
	int patchInitSize = patchMatchSize * 3;
	ObservationFactory obsFact;
	obsFact.addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker()));
	obsFact.addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker()));

	// INIT : 1 map, 2 robs, 3 sens
	//world_ptr_t worldPtr(new WorldAbstract());
	world_ptr_t worldPtr = *world;
	worldPtr->display_mutex.lock();
	map_ptr_t mapPtr(new MapAbstract(307));
	worldPtr->addMap(mapPtr);
	mapPtr->clear();
	robodo_ptr_t robPtr1(new RobotOdometry(mapPtr));
	robPtr1->id(robPtr1->robotIds.getId());
	robPtr1->linkToParentMap(mapPtr);
	vec v(robPtr1->mySize());
	fillVector(v, 0.0);
	robPtr1->state.x(v);
	robPtr1->pose.x(quaternion::originFrame());
	robPtr1->dt_or_dx = 0.1;
	v.resize(robPtr1->mySize_perturbation());
	fillVector(v, 0.01);
	robPtr1->perturbation.clear();
	robPtr1->perturbation.std(v);
	pinhole_ptr_t senPtr11 (new SensorPinHole(robPtr1, MapObject::UNFILTERED));
	senPtr11->id(senPtr11->sensorIds.getId());
	senPtr11->linkToParentRobot(robPtr1);
	senPtr11->state.clear();
	senPtr11->pose.x(quaternion::originFrame());
	senPtr11->params.setImgSize(imgWidth, imgHeight);
	senPtr11->params.setIntrinsicCalibration(k, d, c);
	senPtr11->params.setMiscellaneous(1.0, 1.0, 9);
//	pinhole_ptr_t senPtr12 (new SensorPinHole(robPtr1, MapObject::FILTERED));
//	senPtr12->id(senPtr12->sensorIds.getId());
//	senPtr12->linkToParentRobot(robPtr1);
//	senPtr12->state.clear();
//	senPtr12->pose.x(quaternion::originFrame());
//	senPtr12->set_parameters(imSz, k, d, c);
//	robodo_ptr_t robPtr2(new RobotOdometry(mapPtr));
//	robPtr2->id(robPtr2->robotIds.getId());
//	robPtr2->linkToParentMap(mapPtr);
//	robPtr2->state.clear();
//	robPtr2->pose.x(quaternion::originFrame());
//	v.resize(6);
//	fillVector(v, 0.1);
//	robPtr2->control = v;
//	pinhole_ptr_t senPtr21 (new SensorPinHole(robPtr2, MapObject::FILTERED));
//	senPtr21->id(senPtr21->sensorIds.getId());
//	senPtr21->linkToParentRobot(robPtr2);
//	senPtr21->state.clear();
//	senPtr21->pose.x(quaternion::originFrame());
//	senPtr21->set_parameters(imSz, k, d, c);

	// Show empty map
	cout << *mapPtr << endl;

	worldPtr->display_mutex.unlock();


	// INIT : complete observations set
	// loop all sensors
	// loop all lmks
	// create sen--lmk observation
	// Temporal loop

	const int NFRAME = 1000;

	kernel::Chrono chrono;
	kernel::Chrono total_chrono;
	kernel::Chrono mutex_chrono;
	int dt, max_dt = 0;
	for (int t = 1; t <= NFRAME; t++) {
//		sleep(1);

		worldPtr->display_mutex.lock();
		//cout << "\n************************************************** " << endl;
		//cout << "\n                 FRAME : " << t << " (blocked " << mutex_chrono.elapsedMicrosecond() << " us)" << endl;
		chrono.reset();


		// foreach robot
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++)
		{
			robot_ptr_t robPtr = *robIter;

//			cout << "\n================================================== " << endl;
//			cout << *robPtr << endl;

			// foreach sensor
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++)
			{
				sensor_ptr_t senPtr = *senIter;
//				cout << "\n________________________________________________ " << endl;
//				cout << *senPtr << endl;

				// get raw-data
				senPtr->acquireRaw() ; // FIXME acquireRaw should not be in the rtslam lib

				// move the filter time to the data raw
				vec u(robPtr->mySize_control()); // TODO put some real values in u.
				fillVector(u, 0.0);
				robPtr->set_control(u);
				robPtr->move();
				
				
				asGrid.renew();
				// 1. Observe known landmarks
				// foreach observation
				int numObs = 0;
				for (SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter != senPtr->observationList().end(); obsIter++)
				{
					observation_ptr_t obsPtr = *obsIter;

					obsPtr->clearEvents();

					// 1a. project
					obsPtr->project();

					// 1b. check visibility
					obsPtr->predictVisibility();
					if (obsPtr->isVisible()){

						// update counter
						obsPtr->counters.nSearch++;

						// Add to tesselation grid for active search
						asGrid.addPixel(obsPtr->expectation.x());

						// 1c. predict appearance
						obsPtr->predictAppearance();

						// 1d. search appearence in raw
						//obsPtr->matchFeature(senPtr->getRaw()) ;
						cv::Rect roi(10,10,100,100); // TODO set with ellipse bounding box
						senPtr->getRaw()->match(RawAbstract::ZNCC, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);

						// 1e. if feature is found
						if (obsPtr->getMatchScore()>0.80) {
							obsPtr->counters.nMatch++;
							obsPtr->events.matched = true;
							obsPtr->computeInnovation() ;

							// 1f. if feature is inlier
							if (obsPtr->compatibilityTest(3.0)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
								obsPtr->counters.nInlier++;
								obsPtr->update() ;
								obsPtr->events.updated = true;
							} // obsPtr->compatibilityTest(3.0)
						} // obsPtr->getScoreMatchInPercent()>80
					} // obsPtr->isVisible()

//					cout << "\n-------------------------------------------------- " << endl;
//					cout << *obsPtr << endl;

					numObs ++;
					if (numObs >= 12) break;
				} // foreach observation


				//cout << chrono.elapsedMicrosecond() << " us ; observed lmks: " << numObs << endl;




				// 2. init new landmarks
				#if 0
				for (LandmarkFactoryList::iterator it = senPtr->lmkFactories.begin(); it != senPtr->lmkFactories.end(); ++it)
				{
					if (mapPtr->unusedStates(it->size())) 
					{
					
					}
				}
				
				
				#endif
				if (mapPtr->unusedStates(LandmarkAnchoredHomogeneousPoint::size())) {

					ROI roi;
					if (asGrid.getROI(roi)){

						//feature_ptr_t featPtr(new FeatureAbstract(2));
						//feature_ptr_t featPtr = obsFact.createFeat(
						feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(patchInitSize,patchInitSize,CV_8U));
						if (senPtr->getRaw()->detect(RawAbstract::HARRIS, featPtr, &roi)) {
//							cout << "\n-------------------------------------------------- " << endl;
//							cout << "Detected pixel: " << featPtr->state.x() << endl;

							// 2a. create lmk object
							ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr)); // add featImgPnt in constructor
							lmkPtr->id(lmkPtr->landmarkIds.getId());
							lmkPtr->linkToParentMap(mapPtr);
//							cout << "Initializing lmk " << lmkPtr->id() << endl;

							// 2b. create obs object
							// todo make lmk creation dynamic with factories or switch or other.
							observation_ptr_t obsPtr = obsFact.create(senPtr,lmkPtr);

							// 2c. fill data for this obs
							obsPtr->events.visible = true;
							obsPtr->events.measured = true;
							//vec measNoiseStd(2); fillVector(measNoiseStd, 1.0);
							//obsPtr->ObservationAbstract::setup(measNoiseStd, ObservationPinHoleAnchoredHomogeneousPoint::getPrior());
							obsPtr->measurement.x(featPtr->measurement.x());

							// 2d. compute and fill stochastic data for the landmark
							obsPtr->backProject();

							// 2e. Create lmk descriptor
							vec7 globalSensorPose = senPtr->globalPose();
							desc_img_pnt_ptr_t descPtr(new DescriptorImagePoint(featPtr, globalSensorPose, obsPtr));
							lmkPtr->setDescriptor(descPtr);

							// Complete SLAM graph with all other obs
							mapPtr->completeObservationsInGraph(senPtr, lmkPtr);

//							cout << "\n-------------------------------------------------- " << endl;
//							cout << *lmkPtr << endl;
						}
					}
				}

			}
		}

		//cout << "total lmks: " << mapPtr->landmarkList().size() << endl;

		dt = chrono.elapsedMicrosecond();
		if (dt > max_dt) max_dt = dt;
		//cout << "frame time : " << dt << " us" << endl;
		worldPtr->display_mutex.unlock();
		mutex_chrono.reset();

	}
	cout << "average time : " << total_chrono.elapsed()/NFRAME << " ms, max frame time " << max_dt << endl;

	
	std::cout << "\nFINISHED !" << std::endl;
  sleep(2);

}


void test_slam01_display(world_ptr_t *world)
{
	//(*world)->display_mutex.lock();
	qdisplay::qtMutexLock<kernel::FifoMutex>((*world)->display_mutex);
	display::ViewerQt *viewerQt = static_cast<display::ViewerQt*>((*world)->getDisplayViewer(display::ViewerQt::id()));
	if (viewerQt == NULL)
	{
		viewerQt = new display::ViewerQt();
		(*world)->addDisplayViewer(viewerQt, display::ViewerQt::id());
	}
	viewerQt->bufferize(*world);
	(*world)->display_mutex.unlock();
	
	viewerQt->render();
}


void test_slam01() {
	world_ptr_t worldPtr(new WorldAbstract());
	
//	test_slam01_main(&worldPtr);
	qdisplay::QtAppStart((qdisplay::FUNC)&test_slam01_display,10,(qdisplay::FUNC)&test_slam01_main,-10,100,&worldPtr);
	JFR_DEBUG("Terminated");
}



int main()
{
	test_slam01();
}
