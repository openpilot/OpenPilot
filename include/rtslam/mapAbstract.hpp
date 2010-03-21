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
#include <boost/shared_ptr.hpp>

#include "kernel/IdFactory.hpp"
#include "jmath/jblas.hpp"
#include "rtslam/rtSlam.hpp"

#include "rtslam/gaussian.hpp"
#include "rtslam/kalmanFilter.hpp"


namespace jafar {
	/**
	 * Namespace rtslam for real-time slam module.
	 * \ingroup rtslam
	 */
	namespace rtslam {
		using namespace std;

		// some forward declarations.
		class RobotAbstract;
		class LandmarkAbstract;

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
				 * Mandatory virtual destructor - Map is used as-is, non-abstract by now
				 */
				 inline virtual ~MapAbstract(){}

				/**
				 * Map's indirect array is a function by now.
				 * \return the indirect array of all used states in the map.
				 * TODO: see how to avoid constructing this each time
				 */
				inline jblas::ind_array ia_used_states() {
					jblas::ind_array res;
					res = jmath::ublasExtra::ia_bool(used_states);
					return res;
				}


				/**
				 * EKF engine
				 */
				ExtendedKalmanFilterIndirect filter;

				/**
				 * A set of robots
				 */
				jafar::rtslam::robots_ptr_set_t robots;

				/**
				 * A set of landmarks
				 */
				landmarks_ptr_set_t landmarks;

				jblas::vec & x();
				jblas::sym_mat & P();
				double & x(size_t i);
				double & P(size_t i, size_t j);

				/**
				 * Query about available free space.
				 * \return the number of unused states
				 */
				inline std::size_t unusedStates() const {
					return max_size - current_size;
				}


				/**
				 * Query about available free space
				 * \return true if at least N non-used states
				 */
				inline bool unusedStates(const size_t N) const {
					return (unusedStates() >= N);
				}


				/*
				 * Robot and landmark addition and removal
				 */
				void addRobot(robot_ptr_t _robPtr);
				void deleteRobot(robot_ptr_t _robPtr);

				void addLandmark(landmark_ptr_t _lmkPtr);
				void deleteLandmark(landmark_ptr_t _lmkPtr);

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
		};

	}
}

#endif /* MAPABSTRACT_HPP_ */
