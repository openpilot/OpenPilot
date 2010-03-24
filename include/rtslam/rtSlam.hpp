/**
 * \file rtSlam.hpp
 *
 *  This file defines general typedefs and material for the whole rtslam project.
 *
 *  Created on: 21/03/2010
 *     \author: jsola@laas.fr
 *
 *
 * \ingroup rtslam
 */

#ifndef RTSLAM_HPP_
#define RTSLAM_HPP_

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"
#include <map>
#include <list>

namespace jafar {
	namespace rtslam {


		// forward declarations
		class MapAbstract;
		class RobotAbstract;
		class SensorAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;


		//		// Regular Pointers:
		//		typedef MapAbstract* map_ptr_t;
		//		typedef RobotAbstract* robot_ptr_t;
		//		typedef SensorAbstract* sensor_ptr_t;
		//		typedef LandmarkAbstract* landmark_ptr_t;
		//		typedef ObservationAbstract* observation_ptr_t;

		// Pointers with boost::shared_ptr:
		typedef boost::shared_ptr<MapAbstract> map_ptr_t;
		typedef boost::shared_ptr<RobotAbstract> robot_ptr_t;
		typedef boost::shared_ptr<SensorAbstract> sensor_ptr_t;
		typedef boost::shared_ptr<LandmarkAbstract> landmark_ptr_t;
		typedef boost::shared_ptr<ObservationAbstract> observation_ptr_t;


		// Pointers with boost::weak_ptr:
		typedef boost::weak_ptr<MapAbstract> map_wkptr_t;
		typedef boost::weak_ptr<RobotAbstract> robot_wkptr_t;
		typedef boost::weak_ptr<SensorAbstract> sensor_wkptr_t;
		typedef boost::weak_ptr<LandmarkAbstract> landmark_wkptr_t;
		typedef boost::weak_ptr<ObservationAbstract> observation_wkptr_t;


		//		// Pointers: upwards
		//		typedef map_ptr_t map_ptr_t;
		//		typedef robot_ptr_t robot_ptr_t;
		//		typedef sensor_ptr_t sensor_ptr_t;
		//		typedef landmark_ptr_t landmark_ptr_t;
		//		typedef observation_ptr_t observation_ptr_t;

		// Pointer sets: downwards
		typedef std::map<size_t, map_ptr_t> map_ptr_set_t;
		typedef std::map<size_t, robot_ptr_t> robots_ptr_set_t;
		typedef std::map<size_t, sensor_ptr_t> sensors_ptr_set_t;
		typedef std::map<size_t, landmark_ptr_t> landmarks_ptr_set_t;
		typedef std::map<size_t, observation_ptr_t> observations_ptr_set_t;

	}
}

#endif /* RTSLAM_HPP_ */
