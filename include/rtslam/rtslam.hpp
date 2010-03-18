/**
 * \file rtslam.hpp
 *
 *  Created on: 17/03/2010
 *     \author: jsola@laas.fr
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef RTSLAM_HPP_
#define RTSLAM_HPP_

//#include "rtslam/mapAbstract.hpp"
//#include "rtslam/robotAbstract.hpp"
//#include "rtslam/sensorAbstract.hpp"
//#include "rtslam/landmarkAbstract.hpp"
//#include "rtslam/objectAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		class MapAbstract;
		class ObjectAbstract;
		class RobotAbstract;
		class SensorAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;

		/**
		 * Base class for linkable objects.
		 *
		 * \author jsola@laas.fr
		 *
		 * This class allows objects to be linked to each other.
		 * All links are defined triangular. See the general rtslam documentation.
		 *
		 * \ingroup rtslam
		 */
		class LinkObject {
			public:
				typedef std::map<size_t, LinkObject*> linkset_t;

				~LinkObject(){}

				linkset_t uplink;
				linkset_t downlink;
		};


		/**
		 * Class for managing all slam objects in rtslam
		 *
		 * \author jsola@laas.fr
		 *
		 * This class serves to keep all parental links in rtslam up to date.
		 * It permits, for instance, the creation of the necessary observation objects
		 * (one per existing sensor) each time a new landmark is added to the map.
		 *
		 * \ingroup rtslam
		 */
		class RtSlam {
			public:
				typedef map<size_t, MapAbstract*> maps_t;
				typedef map<size_t, RobotAbstract*> robots_t;
				typedef map<size_t, SensorAbstract*> sensors_t;
				typedef map<size_t, LandmarkAbstract*> landmarks_t;
				typedef map<size_t, ObservationAbstract*> observations_t;

				// Unary - link to RtSlam object
				template<class Obj>
				void add(Obj * objPtr);
				template<class Obj>
				void remove(Obj * objPtr);

				// Binary - link object pairs
				template<class ObjU, class ObjL>
				void link(ObjU * objPtrUpper, ObjL * objPtrLower);
				template<class ObjU, class ObjL>
				void unlink(ObjU * objPtrUpper, ObjL * objPtrLower);

		};

	}
}

#endif /* RTSLAM_HPP_ */
