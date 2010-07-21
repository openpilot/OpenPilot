/**
 * landmarkAnchoredHomogeneousPoint.cpp
 *
 * \date 08/03/2010
 * \author jsola@laas.fr
 *
 *  \file landmarkAnchoredHomogeneousPoint.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"

namespace jafar {
	namespace rtslam {


		/**
		 * Constructor from map
		 */
		LandmarkAnchoredHomogeneousPoint::LandmarkAnchoredHomogeneousPoint(const map_ptr_t & mapPtr) :
			LandmarkAbstract(mapPtr, 7) {
			geomType = POINT,
			type = PNT_AH;
		}

		LandmarkAnchoredHomogeneousPoint::LandmarkAnchoredHomogeneousPoint(const simulation_t dummy, const map_ptr_t & mapPtr) :
			LandmarkAbstract(FOR_SIMULATION, mapPtr, 7) {
			geomType = POINT,
			type = PNT_AH;
		}

		bool LandmarkAnchoredHomogeneousPoint::needToDie(DecisionMethod dieMet){
			double rho = state.x(6);
			if (rho < 0)
			{
				cout << "Lmk AHP " << id() << " Killed by negative depth (" << rho << ")"<< endl;
				return true;
			}
			return LandmarkAbstract::needToDie(dieMet);
		}

	} // namespace rtslam
} // namespace jafar
