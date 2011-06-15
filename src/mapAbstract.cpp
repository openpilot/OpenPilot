/**
 * \file mapAbstract.cpp
 * \date 10/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "jmath/indirectArray.hpp"
#include <boost/shared_ptr.hpp>
#include "jmath/random.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/observationAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		// Serializer function very long: and defined at the end of file
		// std::ostream& operator <<(std::ostream & s, MapAbstract const & map) {

		/**
		 * Constructor
		 */
		MapAbstract::MapAbstract(size_t _max_size) :
			state(7), max_size(_max_size), current_size(0), used_states(max_size) {
			used_states.clear();
			filterPtr.reset(new ExtendedKalmanFilterIndirect(_max_size));
		}
		MapAbstract::MapAbstract(const ekfInd_ptr_t & ekfPtr) :
			state(7), filterPtr(ekfPtr), max_size(ekfPtr->size()), current_size(0),
			    used_states(ekfPtr->size()) {
			used_states.clear();
		}

		jblas::vec & MapAbstract::x() {
			return filterPtr->x();
		}
		jblas::sym_mat & MapAbstract::P() {
			return filterPtr->P();
		}
		double & MapAbstract::x(size_t i) {
			return filterPtr->x(i);
		}
		double & MapAbstract::P(size_t i, size_t j) {
			return filterPtr->P(i, j);
		}

		jblas::ind_array MapAbstract::reserveStates(const std::size_t N) {
			if (unusedStates(N)) {
				jblas::ind_array res = jmath::ublasExtra::ia_pushfront(used_states, N);
				current_size += N;
				return res;
			} else {
				jblas::ind_array res(0);
				return res;
			}
		}

		jblas::ind_array MapAbstract::convertStates(const jblas::ind_array & _ia,
		    const std::size_t N, jblas::ind_array & _icomp) {
			if (_ia.size() < N) {
				std::cerr << __PRETTY_FUNCTION__ << "(#" << __LINE__
				    << "): initial size unsufficient." << std::endl;
				throw "convertStates: size unsufficient.";
			}
			if (_icomp.size() + N != _ia.size()) {
				std::cerr << __PRETTY_FUNCTION__ << "(#" << __LINE__
				    << "): initial sizes not consistent (" << _icomp.size() << "+" << N
				    << "!=" << _ia.size() << ")." << std::endl;
				throw "convertStates: sizes inconsistent.";
			}
			jblas::ind_array res(N);
			for (size_t i = 0; i < N; ++i) {
				res(i) = _ia(i);
			}
			for (size_t i = N; i < _ia.size(); ++i) {
				_icomp(i - N) = _ia(i);
			}
			return res;
		}

		void MapAbstract::liberateStates(const jblas::ind_array & _ia) {
			for (size_t i = 0; i < _ia.size(); i++) {
				int j = _ia(i);
				if (used_states(j) == true) {
					used_states(j) = false;
					current_size -= 1;
				}
			}
		}


		void MapAbstract::clear() {
			x().clear();
			P().clear();
		}

		void MapAbstract::fillSeq() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				for (size_t j = 0; j < max_size; j++)
					P(i, j) = i + 100 * j;
			}
		}

		void MapAbstract::fillDiag() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				P(i, i) = 1;
			}
		}

		void MapAbstract::fillDiagSeq() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				P(i, i) = i;
			}
		}

		void MapAbstract::fillRndm() {
			randVector(x());
			randMatrix(P());
		}

		/**
		 * Serializer. Print all MAP data.
		 *
		 * It traverses the map tree in the following way:
		 * - robots
		 *   - sensors in robot
		 * - landmarks
		 *   - observations of landmark from each sensor
		 */
		std::ostream& operator <<(std::ostream & s,
		    MapAbstract const & map) {

			s << "\n% ROBOTS AND SENSORS \n%=========================" << endl;
			for (MapAbstract::RobotList::const_iterator robIter =
			    map.robotList().begin(); robIter != map.robotList().end(); robIter++) {
				robot_ptr_t robPtr = *robIter;
				s << *robPtr << endl << endl;
				for (RobotAbstract::SensorList::const_iterator senIter =
				    robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++) {
					sensor_ptr_t senPtr = *senIter;
					s << *senPtr << endl << endl;;
				}
			}
			s << "\n% LANDMARKS AND OBSERVATIONS \n%=========================="
			    << endl;
			for (MapAbstract::MapManagerList::const_iterator mmIter =
			    map.mapManagerList().begin(); mmIter != map.mapManagerList().end(); mmIter++) {
				const map_manager_ptr_t mm = *mmIter;
				for (MapManagerAbstract::LandmarkList::const_iterator lmkIter =
				    mm->landmarkList().begin(); lmkIter != mm->landmarkList().end(); lmkIter++) {
					landmark_ptr_t lmkPtr = *lmkIter;
					s << *lmkPtr << endl;
					for (LandmarkAbstract::ObservationList::iterator obsIter =
					    lmkPtr->observationList().begin(); obsIter
					    != lmkPtr->observationList().end(); obsIter++) {
						observation_ptr_t obsPtr = *obsIter;
						s << *obsPtr << endl;
					}
				}
			}
			return s;
		}

		void MapAbstract::writeLogHeader(kernel::DataLogger& log) const
		{
			//std::ostringstream oss; oss << "Map";
			//log.writeComment(oss.str());
			//log.writeLegend("time");
		}
		
		void MapAbstract::writeLogData(kernel::DataLogger& log) const
		{
			//log.writeData(filterPtr);
		}


	}
}
