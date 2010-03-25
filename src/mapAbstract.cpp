/**
 * mapAbstract.cpp
 *
 *  Created on: 10/03/2010
 *      Author: jsola
 *
 *  \file mapAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/indirectArray.hpp"
#include <boost/shared_ptr.hpp>

#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

// TODO this needs to go out of here - when we'll have factories
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"


namespace jafar {
	namespace rtslam {
		using namespace std;


		/**
		 * Constructor
		 */
		MapAbstract::MapAbstract(size_t _max_size) :
			max_size(_max_size), current_size(0), used_states(max_size), filter(max_size) {
			used_states.clear();
		}

		jblas::vec & MapAbstract::x() {
			return filter.x();
		}
		jblas::sym_mat & MapAbstract::P() {
			return filter.P();
		}
		double & MapAbstract::x(size_t i) {
			return filter.x(i);
		}
		double & MapAbstract::P(size_t i, size_t j) {
			return filter.P(i, j);
		}


		/**
		 * Robot addition
		 */
		void MapAbstract::linkToRobot(robot_ptr_t _robPtr) {
			robots[_robPtr->id()] = _robPtr;
		}


		/**
		 * Landmark addition
		 */
		void MapAbstract::linkToLandmark(landmark_ptr_t _lmkPtr) {
			landmarks[_lmkPtr->id()] = _lmkPtr;
		}

		jblas::ind_array MapAbstract::reserveStates(const std::size_t N) {
			if (unusedStates(N)) {
				jblas::ind_array res = jmath::ublasExtra::ia_pushfront(used_states, N);
				current_size += N;
				return res;
			}
			else {
				jblas::ind_array res(0);
				return res;
			}
		}


		/**
		 * Liberate the space indicated.
		 * \param _ia the space to liberate.
		 */
		void MapAbstract::liberateStates(const jblas::ind_array & _ia) {
			for (size_t i = 0; _ia.size(); i++)
				if (used_states(_ia(i)) == true) {
					used_states(_ia(i)) = false;
					current_size += 1;
				}
		}

		void MapAbstract::addObservations(landmark_ptr_t lmkPtr) {
			for (robots_ptr_set_t::iterator robIter = robots.begin(); robIter != robots.end(); robIter++) {
				robot_ptr_t robPtr = robIter->second;
				for (sensors_ptr_set_t::iterator senIter = robPtr->sensors.begin(); senIter != robPtr->sensors.end(); senIter++) {
					sensor_ptr_t senPtr = senIter->second;
					observation_ptr_t obsPtr = newObservation(senPtr, lmkPtr);
				}
			}
		}

		observation_ptr_t MapAbstract::newObservation(sensor_ptr_t senPtr, landmark_ptr_t lmkPtr){
			boost::shared_ptr<ObservationPinHoleAnchoredHomogeneousPoint> obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint());
			//	obsPtr->id() = 0;
			obsPtr->id() = 1000 * senPtr->id() + lmkPtr->id();
			obsPtr->linkToSensor(senPtr);
			obsPtr->linkToLandmark(lmkPtr);
			senPtr->linkToObservation(obsPtr);
			lmkPtr->linkToObservation(obsPtr);

			return obsPtr;
		}

	}
}
