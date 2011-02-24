#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"

namespace jafar {
   namespace rtslam {


      /**
       * Constructor from map
       */
      LandmarkAnchoredHomogeneousPointsLine::LandmarkAnchoredHomogeneousPointsLine(const map_ptr_t & mapPtr) :
         LandmarkAbstract(mapPtr, 11) {
         geomType = LINE,
         type = LINE_AHPL;
         converged = false;
      }

      LandmarkAnchoredHomogeneousPointsLine::LandmarkAnchoredHomogeneousPointsLine(const simulation_t dummy, const map_ptr_t & mapPtr) :
         LandmarkAbstract(FOR_SIMULATION, mapPtr, 11) {
         geomType = LINE,
         type = LINE_AHPL;
         converged = false;
      }

      bool LandmarkAnchoredHomogeneousPointsLine::needToDie(DecisionMethod dieMet){
         double rho = state.x(6);
         if (rho < 0)
         {
            JFR_DEBUG( "Lmk AHP " << id() << " Killed by negative depth (" << rho << ")" );
            return true;
         }
         return LandmarkAbstract::needToDie(dieMet);
      }

   } // namespace rtslam
} // namespace jafar
