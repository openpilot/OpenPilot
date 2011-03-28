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

      LandmarkAnchoredHomogeneousPointsLine::LandmarkAnchoredHomogeneousPointsLine(const map_ptr_t & _mapPtr, const landmark_ptr_t _prevLmk,jblas::ind_array & _icomp) :
         LandmarkAbstract(_mapPtr, 11) {
          // Do nothing this constructor is not supposed to be called
          // Nothing reparametrizes to AHPL (yet)
          //assert(false);
      }

      bool LandmarkAnchoredHomogeneousPointsLine::needToDie(){
         double rho1 = state.x(6);
         double rho2 = state.x(10);
         if (rho1 < 0 || rho2 < 0)
         {
            JFR_DEBUG( "Lmk AHP " << id() << " Killed by negative depth (" << rho1 << " " << rho2 << ")" );
            return true;
         }
         return LandmarkAbstract::needToDie();
      }

   } // namespace rtslam
} // namespace jafar
