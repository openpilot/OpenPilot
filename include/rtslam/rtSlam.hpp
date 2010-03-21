/**
 * \file rtSlam.hpp
 *
 *  Created on: 21/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  This file defines general typedefs and material for the whole rtslam project.
 *
 * \ingroup rtslam
 */



#ifndef RTSLAM_HPP_
#define RTSLAM_HPP_

#include "boost/shared_ptr.hpp"
#include <map>


namespace jafar {
	namespace rtslam {

		// forward declarations
		class ObjectAbstract;
		class MapAbstract;
		class RobotAbstract;
		class SensorAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;

		//	typedef ObjectAbstract* object_ptr_t; // this and all others is if we used regular pointers
		// These with boodt::shared_ptr:
		typedef boost::shared_ptr<ObjectAbstract> object_ptr_t;
		typedef boost::shared_ptr<MapAbstract> map_ptr_t;
		typedef boost::shared_ptr<RobotAbstract> robot_ptr_t;
		typedef boost::shared_ptr<SensorAbstract> sensor_ptr_t;
		typedef boost::shared_ptr<LandmarkAbstract> landmark_ptr_t;
		typedef boost::shared_ptr<ObservationAbstract> observation_ptr_t;

		// Pointer sets
		typedef std::map<size_t, object_ptr_t> objects_ptr_set_t;
		typedef std::map<size_t, map_ptr_t> map_ptr_set_t;
		typedef std::map<size_t, robot_ptr_t> robots_ptr_set_t;
		typedef std::map<size_t, sensor_ptr_t> sensors_ptr_set_t;
		typedef std::map<size_t, landmark_ptr_t> landmarks_ptr_set_t;
		typedef std::map<size_t, observation_ptr_t> observations_ptr_set_t;

	}
}

#endif /* RTSLAM_HPP_ */
