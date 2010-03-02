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
		class MapAbstract {

			public:

				/**
				 * Size things and map usage management
				 */
				size_t max_size;
				size_t current_size;
				vector<bool> used_states;
				jblas::ind_array * ia;

				/**
				 * Constructor
				 */
				MapAbstract(size_t max_size);

				/**
				 * Gaussian map
				 */
				Gaussian gaussian;

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
				 * \param _size the requested free space size.
				 * \return the resulting free space.
				 */
				jblas::ind_array getFreeSpace(const size_t _size);

				/**
				 * Liberate the space indicated.
				 * \param _ia the space to liberate.
				 */
				void liberateSpace(const jblas::ind_array & _ia);

				/**
				 * Robot and landmark addition and removal
				 */
				void addRobot(RobotAbstract& _rob);
				void removeRobot(RobotAbstract& _rob);
				void addLandmark(LandmarkAbstract& _lmk);
				void removeLandmark(LandmarkAbstract& _lmk);

				/**
				 * Full map operations
				 */
				void predictMotion(RobotAbstract & _rob);
				void stackOneLmkCorrection(LandmarkAbstract & _lmk, ObservationAbstract & _obs);
				void correctStackedLmks();
				void correctOneLmk(LandmarkAbstract & _lmk, ObservationAbstract & _obs);
				void reparametrizeLmk(LandmarkAbstract & _lmk);
				void initializeLmk(ObservationAbstract & _obs);
				void deleteLmk(LandmarkAbstract & _lmk);
		};

	}
}

#endif /* MAPABSTRACT_HPP_ */
