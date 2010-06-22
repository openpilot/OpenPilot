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
#include "rtslam/activeSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"

#include "rtslam/hardwareSensorCameraFirewire.hpp"

#include "rtslam/display_qt.hpp"
//#include "image/Image.hpp"

//#include <map>

using namespace jblas;
using namespace jafar;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;


void demo_slam01_main(world_ptr_t *world) {

	const int MAPSIZE = 200;
	const int NFRAME = 5000;
	const int NUPDATES = 1000;

	const double FRAMERATE = 60;

	int imgWidth = 640, imgHeight = 480;
	ActiveSearchGrid asGrid(imgWidth, imgHeight, 4, 4, 16, 5);
	double _d[3] = {-0.27965, 0.20059, -0.14215};
//	double _d[0];
	vec d = createVector<3>(_d);
	double _k[4] = {551.379, 365.793, 1079.2, 1076.73};
//	double _k[4] = {320, 240, 320, 320};
	vec k = createVector<4>(_k);
	int patchMatchSize = 7;
	ObservationFactory obsFact;
	obsFact.addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeEucpObservationMaker()));
	obsFact.addMaker(boost::shared_ptr<ObservationMakerAbstract>(new PinholeAhpObservationMaker()));

	// INIT : 1 map, 2 robs, 3 sens
	//world_ptr_t worldPtr(new WorldAbstract());
	world_ptr_t worldPtr = *world;
	worldPtr->display_mutex.lock();
	
	// create maps
	map_ptr_t mapPtr(new MapAbstract(MAPSIZE));
	worldPtr->addMap(mapPtr);
	mapPtr->clear();
	
	// create robots
	robconstvel_ptr_t robPtr1(new RobotConstantVelocity(mapPtr));
	robPtr1->setId();
	robPtr1->linkToParentMap(mapPtr);
	vec v(robPtr1->mySize());
	fillVector(v, 0.0);
	robPtr1->state.x(v);
	robPtr1->pose.x(quaternion::originFrame());
	robPtr1->dt_or_dx = 1/FRAMERATE;
	v.resize(robPtr1->mySize_perturbation());
	fillVector(v, 1.0);
	robPtr1->perturbation.clear();
	robPtr1->perturbation.set_std_continuous(v);
	robPtr1->constantPerturbation = false;
//	robPtr1->perturbation.set_P_from_continuous(robPtr1->dt_or_dx);
//	robPtr1->computeStatePerturbation();
//	cout << "pert. " << robPtr1->perturbation << "\n .Q: " << robPtr1->Q << endl;
	
	// create sensors
	pinhole_ptr_t senPtr11 (new SensorPinHole(robPtr1, MapObject::UNFILTERED));
	senPtr11->setId();
	senPtr11->linkToParentRobot(robPtr1);
	senPtr11->state.clear();
	senPtr11->pose.x(quaternion::originFrame());
	senPtr11->params.setImgSize(imgWidth, imgHeight);
	senPtr11->params.setIntrinsicCalibration(k, d, d.size());
	senPtr11->params.setMiscellaneous(1.0, 0.1, patchMatchSize);

	viam_hwmode_t hwmode = { VIAM_HWSZ_640x480, VIAM_HWFMT_MONO8, VIAM_HW_FIXED, VIAM_HWFPS_60, VIAM_HWTRIGGER_INTERNAL };
	// UNCOMMENT THESE TWO LINES TO ENABLE FIREWIRE CAMERA OPERATION
	hardware_sensor_ptr_t hardSen11(new HardwareSensorCameraFirewire("0x00b09d01006fb38f", hwmode));
	senPtr11->setHardwareSensor(hardSen11);
	
	cout << "d: " << senPtr11->params.distortion << "\nc: " << senPtr11->params.correction << endl;


	// Show empty map
	cout << *mapPtr << endl;

	worldPtr->display_mutex.unlock();


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
//		sleep(1);
		bool had_data = false;

		worldPtr->display_mutex.lock();
//		cout << "\n************************************************** " << endl;
//		cout << "\n                 FRAME : " << t << " (blocked " << mutex_chrono.elapsedMicrosecond() << " us)" << endl;
		chrono.reset();


		// foreach robot
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++)
		{
			robot_ptr_t robPtr = *robIter;

//			cout << "\n================================================== " << endl;
//			cout << *robPtr << endl;

			// foreach sensor
//kernel::Chrono chronototal;
			for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++)
			{
				sensor_ptr_t senPtr = *senIter;
//				cout << "\n________________________________________________ " << endl;
//				cout << *senPtr << endl;

				// get raw-data
				if (senPtr->acquireRaw() < 0) continue;
//std::cout << chronototal.elapsed() << " has acquired" << std::endl;

				// move the filter time to the data raw
				vec u(robPtr->mySize_control()); // TODO put some real values in u.
				fillVector(u, 0.0);
//				robPtr->set_control(u);
				robPtr->move(u, senPtr->getRaw()->timestamp);
				
				asGrid.renew();
				
				// 1. Observe known landmarks
				// foreach observation
				int numObs = 0;
				observation_ptr_t obsPtr;
int total_match_time = 0;
				for (SensorAbstract::ObservationList::iterator obsIter = senPtr->observationList().begin(); obsIter != senPtr->observationList().end(); obsIter++)
				{
//std::cout << chronototal.elapsed() << " process observation" << std::endl;
					obsPtr = *obsIter;

					obsPtr->clearEvents();
					obsPtr->measurement.matchScore = 0;

					// 1a. project
					obsPtr->project();
//std::cout << chronototal.elapsed() << " has projected" << std::endl;

					// Add to tesselation grid for active search
					asGrid.addPixel(obsPtr->expectation.x());

					// 1b. check visibility
					obsPtr->predictVisibility();
					if (obsPtr->isVisible()){

						numObs ++;
						if (numObs <= NUPDATES){

						// update counter
						obsPtr->counters.nSearch++;

						// 1c. predict appearance
						obsPtr->predictAppearance();
//std::cout << chronototal.elapsed() << " has predicted appearance" << std::endl;

						// 1d. search appearence in raw
						//obsPtr->matchFeature(senPtr->getRaw()) ;
						int xmin, xmax, ymin, ymax;
						double dx, dy;
						dx = 3.0*sqrt(obsPtr->expectation.P(0,0));
						dy = 3.0*sqrt(obsPtr->expectation.P(1,1));
						xmin = (int)(obsPtr->expectation.x(0)-dx);
						xmax = (int)(obsPtr->expectation.x(0)+dx+0.9999);
						ymin = (int)(obsPtr->expectation.x(1)-dy);
						ymax = (int)(obsPtr->expectation.x(1)+dy+0.9999);

						cv::Rect roi(xmin,ymin,xmax-xmin+1,ymax-ymin+1);

//kernel::Chrono chronomatch;
						senPtr->getRaw()->match(RawAbstract::ZNCC, obsPtr->predictedAppearance, roi, obsPtr->measurement, obsPtr->observedAppearance);
//total_match_time += chronomatch.elapsedMicrosecond();
//std::cout << chronototal.elapsed() << " has matched" << std::endl;
/*
						// DEBUG: save some appearances to file
						((AppearanceImagePoint*)(((DescriptorImagePoint*)(obsPtr->landmark().descriptorPtr.get()))->featImgPntPtr->appearancePtr.get()))->patch.save("descriptor_app.png");
						((AppearanceImagePoint*)(obsPtr->predictedAppearance.get()))->patch.save("predicted_app.png");
						((AppearanceImagePoint*)(obsPtr->observedAppearance.get()))->patch.save("matched_app.png");

						// DEBUG: display predicted appearances on image, disable it when operating normally because can have side effects
						AppearanceImagePoint* appImgPtr = PTR_CAST<AppearanceImagePoint*>(obsPtr->predictedAppearance.get());
						jblas::veci shift(2); shift(0) = (appImgPtr->patch.width()-1)/2; shift(1) = (appImgPtr->patch.height()-1)/2;
						appImgPtr->patch.robustCopy(*PTR_CAST<RawImage*>(senPtr->getRaw().get())->img, 0, 0, obsPtr->expectation.x(0)-shift(0), obsPtr->expectation.x(1)-shift(1));

						// DEBUG: some display
						cout << obsPtr->expectation.x() << endl;
						cout << roi << endl;
						cout << obsPtr->getMatchScore() << endl;
*/
						// 1e. if feature is found
						if (obsPtr->getMatchScore()>0.95) {
							obsPtr->counters.nMatch++;
							obsPtr->events.matched = true;
							obsPtr->computeInnovation() ;

							// 1f. if feature is inlier
							if (obsPtr->compatibilityTest(3.0)) { // use 3.0 for 3-sigma or the 5% proba from the chi-square tables.
								obsPtr->counters.nInlier++;
								obsPtr->update() ;
//std::cout << chronototal.elapsed() << " has updated" << std::endl;

								obsPtr->events.updated = true;
							} // obsPtr->compatibilityTest(3.0)
						} // obsPtr->getScoreMatchInPercent()>80
						} // number of observations
					} // obsPtr->isVisible()

//					cout << "\n-------------------------------------------------- " << endl;
//					cout << *obsPtr << endl;

				} // foreach observation
//std::cout << "all matches took " << total_match_time/1000 << " ms" << std::endl;

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

						feat_img_pnt_ptr_t featPtr(new FeatureImagePoint(patchMatchSize*3,patchMatchSize*3,CV_8U));
//kernel::Chrono chronodetect;
						if (senPtr->getRaw()->detect(RawAbstract::HARRIS, featPtr, &roi)) {
//std::cout << "detect in roi " << roi << " took " << chronodetect.elapsed() << " ms" << std::endl;
//							cout << "\n-------------------------------------------------- " << endl;
//							cout << "Detected pixel: " << featPtr->measurement.x() << endl;

//((AppearanceImagePoint*)(featPtr->appearancePtr.get()))->patch.save("detected_patch.png");
							
							// 2a. create lmk object
							ahp_ptr_t lmkPtr(new LandmarkAnchoredHomogeneousPoint(mapPtr)); // add featImgPnt in constructor
							lmkPtr->setId();
							lmkPtr->linkToParentMap(mapPtr);
//							cout << "Initializing lmk " << lmkPtr->id() << endl;

							// 2b. create obs object
							observation_ptr_t obsPtr = obsFact.create(senPtr,lmkPtr);
							obsPtr->setId();

							// 2c. fill data for this obs
							obsPtr->events.visible = true;
							obsPtr->events.measured = true;
							obsPtr->measurement.x(featPtr->measurement.x());

							// 2d. compute and fill stochastic data for the landmark
							obsPtr->backProject();

							// 2e. Create lmk descriptor
							vec7 globalSensorPose = senPtr->globalPose();
							desc_img_pnt_ptr_t descPtr(new DescriptorImagePoint(featPtr, globalSensorPose, obsPtr));
							lmkPtr->setDescriptor(descPtr);

							// Complete SLAM graph with all other obs
							//mapPtr->completeObservationsInGraph(senPtr, lmkPtr); // FIXME

//							cout << "\n-------------------------------------------------- " << endl;
//							cout << *lmkPtr << endl;
						} // detect()
					} // getROI()
				} // unusedStates()

				had_data = true;

			} // for each sensor
		} // for each robot

		//cout << "total lmks: " << mapPtr->landmarkList().size() << endl;

		dt = chrono.elapsedMicrosecond();
		if (dt > max_dt) max_dt = dt;
		if (had_data) {
			t++;
			vec7 F = robPtr1->pose.x();
			vec3 p = subrange(F, 0, 3);
			vec4 q = subrange(F, 3, 7);
			vec3 e = quaternion::q2e(q);
			cout << "dt: " << (int)(1000*robPtr1->dt_or_dx) << " ms. p: " << 100*p << " cm. e: " << (180.0 / 3.14)*e << " deg." << endl;
		}
		worldPtr->display_mutex.unlock();
		mutex_chrono.reset();

	} // temporal loop

	cout << "average time : " << total_chrono.elapsed()/NFRAME << " ms, max frame time " << max_dt << endl;
	std::cout << "\nFINISHED !" << std::endl;

	sleep(60);

} // demo_slam01_main


void demo_slam01_display(world_ptr_t *world)
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


void demo_slam01() {
	world_ptr_t worldPtr(new WorldAbstract());
	
	// to start with qt display
	const int slam_priority = 0; // needs to be started as root to be > 0
	const int display_priority = 0;
	const int display_period = 100; // ms
	qdisplay::QtAppStart((qdisplay::FUNC)&demo_slam01_display,slam_priority,(qdisplay::FUNC)&demo_slam01_main,display_priority,display_period,&worldPtr);
	// to start without display
	//demo_slam01_main(&worldPtr);

	JFR_DEBUG("Terminated");
}



int main()
{
	demo_slam01();
}
