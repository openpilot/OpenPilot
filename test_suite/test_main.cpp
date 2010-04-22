/**
 * \file test_main.cpp
 *
 * \date 24/03/2010
 * \author jsola@laas.fr
 *
 *
 *  This test acts as a main() function for rtslam.
 *
 *  It is supposed to implement a simple but full SLAM program.
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
#include "rtslam/objectAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/sensorPinHole.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"
#include "rtslam/kalmanFilter.hpp"
#include "rtslam/robotInertial.hpp"

//#include <map>

using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;


/**
 * Add a new robot to map
 * \param mapPtr a pointer to the SLAM map.
 * \param name the name of the robot.
 */
robot_ptr_t newRobot(map_ptr_t & mapPtr, string name) {

	size_t rid = mapPtr->robotIds.getId();
	constvel_ptr_t robPtr(new RobotConstantVelocity(mapPtr));
	robPtr->id(rid);
	robPtr->name(name);
	robPtr->linkToParentMap(mapPtr);
	cout << "    added rob: " << robPtr->id() << endl;
	return robPtr;
}

/**
 * Add a new sensor.
 * \param robPtr a pointer to the robot owning the sensor.
 * \param inInMap flag to estimate robot pose within the SLAM EKF.
 */
sensor_ptr_t newSensor(robot_ptr_t & robPtr, string name, MapObject::filtered_obj_t inFilter = MapObject::UNFILTERED) {
	map_ptr_t mapPtr = robPtr->mapPtr();

	size_t sid = mapPtr->sensorIds.getId();
	pinhole_ptr_t senPtr(new SensorPinHole(robPtr, inFilter));

	senPtr->id(sid);
	senPtr->name(name);
	senPtr->linkToParentRobot(robPtr);
	cout << "    added sen: " << senPtr->id() << endl;
	return senPtr;
}

map_ptr_t initSlam(size_t size_map) {

	using namespace boost;


	// create a map
	shared_ptr<MapAbstract> mapPtr(new MapAbstract(size_map));


	// Map object sizes;
	std::size_t size_robCV = RobotConstantVelocity::size();
	std::size_t size_senPH = SensorPinHole::size();


	// Add 1 robots, 2 sensors
	if (mapPtr->unusedStates(size_robCV)) {
		robot_ptr_t robPtr = newRobot(mapPtr, "SUBMARINE");
		robPtr->pose.x(quaternion::originFrame());
		cout << *robPtr << endl;

		if (mapPtr->unusedStates(size_senPH)) {
			sensor_ptr_t senPtr = newSensor(robPtr, "FLEA", MapObject::UNFILTERED);
			senPtr->pose.x(quaternion::originFrame());
			cout << *senPtr << endl;
		}
		if (mapPtr->unusedStates(size_senPH)) {
			sensor_ptr_t senPtr = newSensor(robPtr, "MARLIN", MapObject::FILTERED);
			senPtr->pose.x(quaternion::originFrame());
			cout << *senPtr << endl;
		}
	}
	if (mapPtr->unusedStates(size_robCV)) {
		robot_ptr_t robPtr = newRobot(mapPtr, "AEROPLANE");
		robPtr->pose.x(quaternion::originFrame());

		if (mapPtr->unusedStates(size_senPH)) {
			sensor_ptr_t senPtr = newSensor(robPtr, "DRAGONFLY", MapObject::UNFILTERED);
			senPtr->pose.x(quaternion::originFrame());
		}
	}
	return mapPtr;
}

/**
 * robot setup.
 * Sets basic date for a robot: initial pose, perturbation levels.
 * \param x0 initial x-coordinate. Same for \a y0, \a z0.
 * \param roll0 initial roll angle. Same for \a pitch0, \a yaw0
 * \param sigma_vi std. dev. of linear velocity impulse.
 * \param sigma_wi std. dev. of angular velocity impulse.
 */
void motionSetup(robot_ptr_t & robPtr, const double x0, const double y0, const double z0, const double roll0,
    const double pitch0, const double yaw0, const double sigma_pos, const double sigma_ori, const double sigma_vi,
    const double sigma_wi) {

	robPtr->state.clear(); // clear state and pose

	vec7 p0; //         initial pose
	sym_mat P0(7); //   pose covariance
	vec3 e; //          euler angles
	vec4 q; //          quaternion
	mat Q_e(4, 3);
	sym_mat Q(4); //    quat cov. mat.

	p0(0) = x0;
	p0(1) = y0;
	p0(2) = z0;

	e(0) = roll0;
	e(1) = pitch0;
	e(2) = yaw0;
	quaternion::e2q(e, q, Q_e);
	subrange(p0, 3, 7) = q;
	Q = ublasExtra::prod_JPJt(ublasExtra::scalar_diag_mat(3, sigma_ori * sigma_ori), Q_e);

	P0.clear();
	subrange(P0, 0, 3, 0, 3) = ublasExtra::scalar_diag_mat(3, sigma_pos * sigma_pos);
	subrange(P0, 3, 7, 3, 7) = Q;

	robPtr->pose.x(p0);
	robPtr->pose.P(P0);

	vec6 std_pert; // Perturbation
	subrange(std_pert, 0, 3) = scalar_vec(3, sigma_vi);
	subrange(std_pert, 3, 6) = scalar_vec(3, sigma_wi);
	robPtr->perturbation.clear();
	robPtr->perturbation.std(std_pert);

	robPtr->computeStatePerturbation();

}

void test_main01() {

	cout << "\n\n\n% ######    WELCOME TO RTSLAM    ######\n" << endl;

	using namespace boost;

	map_ptr_t mapPtr = initSlam(70);
	//	initSomeLmks(mapPtr, 2);
	double dt = 1;
	double t_end = 3;


	// setup: add a robot with 2 sensors
	for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++) {
		robot_ptr_t robPtr = *robIter;

		motionSetup(robPtr, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);
		// set robot control to some easy value
		robPtr->dt_or_dx = dt;
		vec6 pert;
		pert.clear();
		pert(0) = 0.1; // increase X m/s forward.
		pert(5) = 0.1; // increase X rad/s = 60X deg/s left.
		robPtr->perturbation.x(pert);
	}

	for (double t = 0; t < t_end; t += dt) { // start SLAM time loop
		cout << " o-----> Time: " << t << endl;
		for (MapAbstract::RobotList::iterator robIter = mapPtr->robotList().begin(); robIter != mapPtr->robotList().end(); robIter++) { // loop robots

			robot_ptr_t robPtr = *robIter;
			cout << "exploring rob: " << robPtr->id() << endl;
			robPtr->move();
			robPtr->exploreSensors();

		} // end of robots loop
	} // end of time loop

	cout << "% GLOBAL SENSOR POSE \n%====================	" << endl;
	vec7 sg_local, sg_remote;
	mat SG_rs_local(7,7);
	mat SG_rs_remote(7,14);

	mapPtr->ParentOf<RobotAbstract>::display(cout);
	cout << endl;
	cout << "Rob = " << *(mapPtr->robotList()[0]) << endl;
	mapPtr->robotList()[0]->ParentOf<SensorAbstract>::display(cout);
	cout << endl;
	cout << "Sen = " << *mapPtr->robotList()[0]->sensorList()[0] << endl;

	sensor_ptr_t senPtr = mapPtr->robotList()[0]->sensorList()[0];
	cout << "SenPtr = " << *senPtr << endl;

	SensorAbstract & sen = *senPtr.get();
	cout << "sn = " << sen << endl;
	mapPtr->robotList()[0]->sensorList()[0]->globalPose(sg_local, SG_rs_local);
//	senPtr->globalPose(sg_local, SG_rs_local);
	cout << "sg_local  = " << sg_local << endl;
	cout << "SG_rs_local  = " << SG_rs_local << endl;
	cout << "sg_local.ia_rsl = " << mapPtr->robotList()[0]->sensorList()[0]->ia_globalPose << endl;

	mapPtr->robotList()[0]->sensorList()[1]->globalPose(sg_remote, SG_rs_remote);

	cout << "sg_remote = " << sg_remote << endl;
	cout << "SG_rs_remote = " << SG_rs_remote << endl;
	cout << "sg_local.ia_rsl = " << mapPtr->robotList()[0]->sensorList()[1]->ia_globalPose << endl;

	cout << "\n THAT'S ALL, WHAT'S WRONG? " << endl;

}

BOOST_AUTO_TEST_CASE( test_main )
{
	test_main01();
}

