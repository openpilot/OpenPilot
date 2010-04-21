/**
 * test_simu.cpp
 *
 * \date 20/04/2010
 * \author jsola@laas.fr
 *
 *  \file test_simu.cpp
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
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

#include <iostream>

#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/rawImageSimu.hpp"
#include "rtslam/featurePointSimu.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/AppearanceImageSimu.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/DesscriptorImagePointSimu.hpp"

using namespace std;
using namespace jblas;
using namespace jafar;
using namespace jafar::rtslam;
using namespace jafar::jmath;

void test_simu01(void) {

	// abstract types = new concrete for simu
	map_ptr_t      mapPtr(new MapAbstract(100));
	robot_ptr_t    robPtr(new RobotConstantVelocity(mapPtr));
	robPtr->linkToParentMap(mapPtr);
	raw_ptr_t      rawSimu(new RawImageSimu()) ;
	feature_ptr_t  featureSimu(new FeaturePointSimu());
	sensor_ptr_t   senPH(new SensorPinHole(robPtr, true)) ;
	senPH->linkToParentRobot(robPtr);
	app_ptr_t      appSimu(new AppearenceImageSimu()) ;
	landmark_ptr_t lmk(new LandmarkEuclideanPoint(mapPtr)) ;
	lmk->linkToParentMap(mapPtr);
	desc_ptr_t     desc(new DesscriptorImagePointSimu()) ;



}

BOOST_AUTO_TEST_CASE( test_simu )
{
	test_simu01();
}

