/**
 * mapManagerAbstract.cpp
 *
 * \date 10/03/2010
 * \author nmansard@laas.fr
 *
 *  \file mapManagerAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include <boost/shared_ptr.hpp>

#include "rtslam/rtSlam.hpp"
#include "rtslam/mapManager.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/dataManagerAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		observation_ptr_t MapManagerAbstract::createNewLandmark(
		    data_manager_ptr_t dmaOrigin) {
			landmark_ptr_t newLmk = createLandmarkInit();
			newLmk->setId();
			newLmk->linkToParentMapManager(shared_from_this());
			observation_ptr_t resObs;

			for (MapManagerAbstract::DataManagerList::iterator iterDMA =
			    dataManagerList().begin(); iterDMA != dataManagerList().end(); ++iterDMA) {
				data_manager_ptr_t dma = *iterDMA;
				observation_ptr_t newObs =
				    dma->observationFactory()->create(dma->sensorPtr(), newLmk);

				/* Insert the observation in the graph. */
				newObs->linkToParentDataManager(dma);
				newObs->linkToParentLandmark(newLmk);
				newObs->linkToSensor(dma->sensorPtr());
				newObs->linkToSensorSpecific(dma->sensorPtr());
				newObs->setId();

				/* Store for the return the obs corresponding to the dma origin. */
				if (dma == dmaOrigin) resObs = newObs;
			}
			return resObs;
		}

		void MapManagerAbstract::killLandmark(landmark_ptr_t lmkPtr){
			// first unlink all observations
			for (LandmarkAbstract::ObservationList::iterator obsIter =
			    lmkPtr->observationList().begin(); obsIter != lmkPtr->observationList().end(); ++obsIter) {
				observation_ptr_t obsPtr = *obsIter;
				obsPtr->dataManagerPtr()->unregisterChild(obsPtr);
			}
			// liberate map space
			mapPtr()->liberateStates(lmkPtr->state.ia());
			// now unlink landmark
			ParentOf<LandmarkAbstract>::unregisterChild(lmkPtr);
		}


		void MapManagerAbstract::manageMap(void) {
			// foreach lmk
			for (LandmarkList::iterator lmkIter = landmarkList().begin(); lmkIter
			    != landmarkList().end(); ++lmkIter) {
				landmark_ptr_t lmkPtr = *lmkIter;
				if (lmkPtr->needToDie() )
				{
					lmkIter++;
					killLandmark(lmkPtr);
					lmkIter--;
				} else if (0)//TODO-NMSD  lmkPtr->needToReparametrize() )
				{
					lmkIter++;
					reparametrizeLandmark(lmkPtr);
					lmkIter--;
				}
			}
		}

	}
	;
}
;
