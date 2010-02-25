/*
 * mapAbstract.hpp
 *
 *     Project: jafar
 *  Created on: Feb 9, 2010
 *      Author: jsola
 */

/**
 * \file mapAbstract.hpp
 * File defining the abstract map class
 * \ingroup rtslam
 */

#ifndef MAPABSTRACT_HPP_
#define MAPABSTRACT_HPP_

#include <list>

#include "jmath/jblas.hpp"
#include "gaussian.hpp"
#include "robotAbstract.hpp"
#include "landmarkAbstract.hpp"

namespace jafar {

	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {

		/** Base class for all map types defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class MapAbstract: public Gaussian {

				/**
				 * Mandatory virtual destructor
				 */
				virtual ~MapAbstract(void);

				/**
				 * A list of robots
				 */
				std::list<RobotAbstract*> robotsList;

				/**
				 * A list of landmarks
				 */
				std::list<LandmarkAbstract*> landmarksList;

				/**
				 * Obtain free Map space of a given size.
				 * \param size_ the requested free space size
				 * \param freeSpace_ the resulting free space
				 * \return \a true if enough space was found.
				 */
				bool getFreeSpace(const size_t size_, jblas::vec& freeSpace_);

				/**
				 * Robot and landmark addition and removal
				 */
				void addRobot(RobotAbstract& robot_);
				void removeRobot(RobotAbstract& robot_);
				void addLandmark(LandmarkAbstract& landmark_);
				void removeLandmark(LandmarkAbstract& landmark_);

				std::vector<size_t> used; // TODO: see how to implement this "used" feature.

		};

	}
}

#endif /* MAPABSTRACT_HPP_ */
