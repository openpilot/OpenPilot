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

#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

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
		void MapAbstract::addRobot(robot_ptr_t _robPtr) {
			robots[_robPtr->id()] = _robPtr;
		}


		/**
		 * Landmark addition
		 */
		void MapAbstract::addLandmark(landmark_ptr_t _lmkPtr) {
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

	}
}
