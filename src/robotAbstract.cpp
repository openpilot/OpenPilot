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
			MapObject(_map, _size_state),
			pose(state, jmath::ublasExtra::ia_range(0,7)),
			control(_size_control),
			F_r(_size_state, _size_state),
			F_u(_size_state, _size_control)
		{
			categoryName("ROBOT"); // robot is categorized
		}


		/*
		 * Add a sensor to this robot
		 */
		void RobotAbstract::addSensor(sensor_ptr_t _senPtr) {
			sensors[_senPtr->id()] = _senPtr;
		}

		void RobotAbstract::linkToMap(map_ptr_t _mapPtr){
			slamMap = _mapPtr;
		}



	}
}
