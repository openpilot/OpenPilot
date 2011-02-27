/**
 * \file landmarkAnchoredHomogeneousPoint.cpp
 * \date 08/03/2010
 * \author jsola
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
			converged = false;
		}

		LandmarkAnchoredHomogeneousPoint::LandmarkAnchoredHomogeneousPoint(const simulation_t dummy, const map_ptr_t & mapPtr) :
			LandmarkAbstract(FOR_SIMULATION, mapPtr, 7) {
			geomType = POINT,
			type = PNT_AH;
			converged = false;
		}

		bool LandmarkAnchoredHomogeneousPoint::needToDie(){
			double rho = state.x(6);
			if (rho < 0)
			{
				JFR_DEBUG( "Lmk AHP " << id() << " Killed by negative depth (" << rho << ")" );
				return true;
			}
			return LandmarkAbstract::needToDie();
		}

	} // namespace rtslam
} // namespace jafar
