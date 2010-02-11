/*
 * rtsMapAbstract.hpp
 *
 *     Project: jafar
 *  Created on: Feb 9, 2010
 *      Author: jsola
 */

#ifndef RTSMAPABSTRACT_HPP_
#define RTSMAPABSTRACT_HPP_

#include <list>
#include "rtsBlocks.hpp"
#include "jmath/jblas.hpp"
#include "rtsRobotAbstract.hpp"
#include "rtsLandmarkAbstract.hpp"

using namespace jblas;

namespace jafar {

	namespace rtslam {

		/** Base class for all map types defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsMapAbstract: public Gaussian {
				/**
				 * Mandatory virtual destructor
				 */
				virtual ~rtsMapAbstract(void);

				/**
				 * A list of robots
				 */
				list<rtsRobotAbstract> robotsList;

				/**
				 * A list of landmarks
				 */
				list<rtsLandmarkAbstract> landmarksList;

				/**
				 * Obtain free Map space of a given size.
				 */
				bool getFreeSpace(int size_, vec& freeSpace_);

				/**
				 * Robot and landmark addition and removal
				 */
				void addRobot(rtsRobotAbstract robot_);
				void removeRobot(rtsRobotAbstract robot_);
				void addLandmark(rtsLandmarkAbstract landmark_);
				void removeLandmark(rtsLandmarkAbstract landmark_);

				list<int> used; // TODO: see how to implement this "used" feature.

		};

	}
}

#endif /* RTSMAPABSTRACT_HPP_ */
