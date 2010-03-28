/**
 * robotAbstract.cpp
 *
 *  Created on: 08/03/2010
 *      Author: jsola
 *
 *  \file robotAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/robotAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/mapAbstract.hpp"

#include <boost/shared_ptr.hpp>

namespace jafar {
	namespace rtslam {
		using namespace std;


		/*
		 * Operator << for class RobotAbstract.
		 * It shows different information of the robot.
		 */
		ostream& operator <<(ostream & s, RobotAbstract & rob) {
			s << rob.categoryName() << " " << rob.id() << ": ";
			if (rob.name().size() > 0)
				s << rob.name() << ", ";
			s << "of type " << rob.type() << endl;
			s << ".state:  " << rob.state << endl;
			s << ".pose :  " << rob.pose << endl;
			s << ".sens : [";
			sensors_ptr_set_t::iterator senIter;
			for (senIter = rob.sensors.begin(); senIter != rob.sensors.end(); senIter++)
				s << " " << senIter->first << " ";
			s << "]";
			return s;
		}


		/*
		 * Remote constructor from remote map and size of control vector.
		 */
		RobotAbstract::RobotAbstract(MapAbstract & _map, const size_t _size_state, const size_t _size_control) :
			MapObject(_map, _size_state), pose(state, jmath::ublasExtra::ia_range(0, 7)), control(_size_control), XNEW_x(
			    _size_state, _size_state), XNEW_control(_size_state, _size_control), Q(_size_state, _size_state) {
			constantPerturbation = false;
			categoryName("ROBOT"); // robot is categorized
		}


		/*
		 * Add a sensor to this robot
		 */
		void RobotAbstract::linkToSensor(sensor_ptr_t _senPtr) {
			sensors[_senPtr->id()] = _senPtr;
		}

		void RobotAbstract::linkToMap(map_ptr_t _mapPtr) {
			slamMap = _mapPtr;
		}

		void RobotAbstract::move() {
			move_func(); // x = F(x, u); Update Jacobians dxnew/dx and dxnew/du
			if (!constantPerturbation)
				computeStatePerturbation();
			slamMap->filter.predict(slamMap->ia_used_states(), XNEW_x, state.ia(), Q); // P = F*P*F' + Q
		}

		void RobotAbstract::computeStatePerturbation() {
			Q = jmath::ublasExtra::prod_JPJt(control.P(), XNEW_control);
			//			cout << "computed Q" << endl; // TODO: remove this if enough time since 2010/03/28 has passed.
		}

		void RobotAbstract::exploreSensors() {
			for (sensors_ptr_set_t::iterator senIter = sensors.begin(); senIter != sensors.end(); senIter++) {
				sensor_ptr_t senPtr = senIter->second;
				cout << "exploring sen: " << senPtr->id() << endl;

				senPtr->acquireRaw();
				senPtr->processRaw();

			}
		}

	}
}
