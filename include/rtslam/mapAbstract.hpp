/*
 * \file mapAbstract.hpp
 *
 *  Created on: Feb 9, 2010
 *     \author jsola@laas.fr
 *
 * Header file for abstract maps
 *
 * \ingroup rtslam
 */

#ifndef MAPABSTRACT_HPP_
#define MAPABSTRACT_HPP_

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

				/**
				 * Print all MAP data.
				 *
				 * It traverses the map tree in the following way:
				 * - robots
				 *   - sensors in robot
				 * - landmarks
				 *   - observations of landmark from each sensor
				 */
				friend std::ostream& operator <<(std::ostream & s, jafar::rtslam::MapAbstract & map);

			public:

				/**
				 * A set of robots
				 */
				robots_ptr_set_t robotsPtrSet;

				/**
				 * A set of landmarks
				 */
				landmarks_ptr_set_t landmarksPtrSet;

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


				void linkToRobot(const robot_ptr_t & _robPtr); ///< Link to robot
				void deleteRobot(const robot_ptr_t & _robPtr);

				void linkToLandmark(const landmark_ptr_t & _lmkPtr); ///< Link to landmark
				void deleteLandmark(const landmark_ptr_t & _lmkPtr);

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

				/**
				 * Add observations to landmark.
				 * This requires traversing all the map tree and this is why this function goes here.
				 * \param lmkPtr the pointer to the landmark to associate observations to.
				 */
				void addObservations(landmark_ptr_t & lmkPtr);


				void fillSeq() ;

				void fillDiag() ;

				void fillRndm() ;

			protected:

				/**
				 * Create new observation.
				 * Creates a new observation from a sensor and a landmark.
				 * TODO see if this can go to class Observation, possibly with a dedicated constructor.
				 */
				observation_ptr_t newObservation(sensor_ptr_t & senPtr, landmark_ptr_t & lmkPtr);




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
