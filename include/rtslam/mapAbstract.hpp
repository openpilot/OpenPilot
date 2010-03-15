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

#include <map>

#include "kernel/IdFactory.hpp"
#include "jmath/jblas.hpp"
#include "rtslam/gaussian.hpp"
#include "rtslam/kalmanFilter.hpp"

namespace jafar {


	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {


		// some forward declarations.
		class RobotAbstract;
		class LandmarkAbstract;
		class ObservationAbstract;


		/** Base class for all map types defined in the module rtslam.
		 *
		 * \author jsola
		 * \ingroup rtslam
		 */
		class MapAbstract {

			public:
				kernel::IdFactory robotIds;
				kernel::IdFactory sensorIds;
				kernel::IdFactory landmarkIds;

			public:


				/**
				 * Size things and map usage management
				 */
				size_t max_size;
				size_t current_size;
				jblas::vecb used_states;

				/**
				 * Constructor
				 */
				MapAbstract(size_t _max_size);

				/**
				 * Mandatory virtual destructor - Map is used as non-abstract by now
				 */
				// inline virtual ~MapAbstract(void);

				/**
				 * Map's indirect array is a function by now.
				 * \return the indirect array of all used states in the map.
				 * TODO: see how to avoid constructing this each time
				 */
				inline jblas::ind_array ia_used_states(void) {
					jblas::ind_array res;
					res = jmath::ublasExtra::ia_set(used_states);
					return res;
				}


				/**
				 * EKF engine
				 */
				ExtendedKalmanFilterIndirect filter;

				/**
				 * A set of robots
				 */
				std::map<size_t, RobotAbstract*> robots;

				/**
				 * A set of landmarks
				 */
				std::list<LandmarkAbstract*> landmarks;

				/**
				 * Query about available free space.
				 * \return the number of unused states
				 */
				inline std::size_t unusedStates(void) const {
					return max_size - current_size;
				}


				/**
				 * Query about available free space
				 * \return true if at least N non-used states
				 */
				inline bool unusedStates(const size_t N) const {
					return (unusedStates() >= N);
				}


//				/**
//				 * Robot and landmark addition and removal
//				 */
//				void addRobot(RobotAbstract * _robPtr);

				/**
				 * Obtain free Map space of a given size.
				 * The free space in \a used_states and the current size \a current_size are modified accordingly.
				 * Ig not enough space is available, the returned indirect array is of null size.
				 * \param _size the requested free space size.
				 * \return the resulting free space.
				 */
				jblas::ind_array reserveStates(const std::size_t _size);

				/**
				 * Liberate the space indicated.
				 * The free space in \a used_states and the current size \a current_size are modified accordingly.
				 * \param _ia the space to liberate.
				 */
				void liberateStates(const jblas::ind_array & _ia);


				//				void removeRobot(RobotAbstract& _rob);
				//				void addLandmark(LandmarkAbstract& _lmk);
				//				void removeLandmark(LandmarkAbstract& _lmk);

				//				/**
				//				 * Full map operations
				//				 */
				//				void predictMotion(RobotAbstract & _rob);
				//				void stackOneLmkCorrection(LandmarkAbstract & _lmk, ObservationAbstract & _obs);
				//				void correctStackedLmks();
				//				void correctOneLmk(LandmarkAbstract & _lmk, ObservationAbstract & _obs);
				//				void reparametrizeLmk(LandmarkAbstract & _lmk);
				//				void initializeLmk(ObservationAbstract & _obs);
				//				void deleteLmk(LandmarkAbstract & _lmk);
		};

	}
}

#endif /* MAPABSTRACT_HPP_ */
