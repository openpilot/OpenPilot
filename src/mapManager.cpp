/**
 * \file mapManagerAbstract.cpp
 * \date 10/03/2010
 * \author nmansard
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

		/** ***************************************************************************************
			MapManagerAbstract
		******************************************************************************************/
	
		observation_ptr_t MapManagerAbstract::createNewLandmark(data_manager_ptr_t dmaOrigin)
		{
			landmark_ptr_t newLmk = lmkFactory->createInit(mapPtr());
			newLmk->setId();
			newLmk->linkToParentMapManager(shared_from_this());
			observation_ptr_t resObs;

			for (MapManagerAbstract::DataManagerList::iterator
			     iterDMA = dataManagerList().begin();
			     iterDMA != dataManagerList().end(); ++iterDMA)
			{
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

	  void MapManagerAbstract::unregisterLandmark(landmark_ptr_t lmkPtr, bool liberateFilter)
		{
			// first unlink all observations
			for (LandmarkAbstract::ObservationList::iterator
			     obsIter = lmkPtr->observationList().begin();
			     obsIter != lmkPtr->observationList().end(); ++obsIter)
			{
				observation_ptr_t obsPtr = *obsIter;
				obsPtr->dataManagerPtr()->unregisterChild(obsPtr);
			}
			// liberate map space
			if( liberateFilter )
			  mapPtr()->liberateStates(lmkPtr->state.ia());
			// now unlink landmark
			ParentOf<LandmarkAbstract>::unregisterChild(lmkPtr);
		}



    void MapManagerAbstract::reparametrizeLandmark(landmark_ptr_t lmkinit)
		{
			//cout<<__PRETTY_FUNCTION__<<"(#"<<__LINE__<<"): " <<"" << endl;

			// unregister lmk
			unregisterLandmark(lmkinit, false);
			//lmkinit->destroyDisplay(); // cannot do that here, display is using it...

			// Create a new landmark advanced instead of the previous init lmk.
			jblas::ind_array idxComp(lmkFactory->sizeComplement());
			//cout << __PRETTY_FUNCTION__ << "about to create lmkcv." << endl;
			landmark_ptr_t lmkconv = lmkFactory->createConverged(mapPtr(), lmkinit, idxComp);

			// link new landmark
			lmkconv->linkToParentMapManager(shared_from_this());

			// Algebra: 2a. compute the jac and 2b. update the filter.
			// a. Call reparametrize_func()
			const size_t size_init = lmkinit->mySize();
			const size_t size_conv = lmkconv->mySize();
			mat CONV_init(size_conv, size_init);
			vec sinit = lmkinit->state.x();
			vec sconv(size_conv);
			//cout << __PRETTY_FUNCTION__ << "about to call lmk->reparametrize_func()" << endl;
			lmkinit->reparametrize_func(sinit, sconv, CONV_init);

			// b. Call filter->reparametrize().
			//cout << __PRETTY_FUNCTION__ << "about to call filter->reparametrize()" << endl;
			lmkconv->state.x() = sconv;
			mapPtr()->filterPtr->reparametrize(mapPtr()->ia_used_states(),
				CONV_init, lmkinit->state.ia(), lmkconv->state.ia());

			// Transfer info from the old lmk to the new one.
			//cout << __PRETTY_FUNCTION__ << "about to transfer lmk info." << endl;
			lmkconv->transferInfoLmk(lmkinit);

			// Create the cv-lmk set of observations, one per sensor.
			for(LandmarkAbstract::ObservationList::iterator
			    obsIter = lmkinit->observationList().begin();
			    obsIter != lmkinit->observationList().end(); ++obsIter)
			{
				observation_ptr_t obsinit = *obsIter;
				data_manager_ptr_t dma = obsinit->dataManagerPtr();
				sensor_ptr_t sen = obsinit->sensorPtr();

				//cout << __PRETTY_FUNCTION__ << "about to create new obs" << endl;
				observation_ptr_t obsconv = dma->observationFactory()->create(sen, lmkconv);
				obsconv->linkToParentDataManager(dma);
				obsconv->linkToParentLandmark(lmkconv);
				obsconv->linkToSensor(sen);
				obsconv->linkToSensorSpecific(sen);
				// transfer info to new obs
				obsconv->transferInfoObs(obsinit);
			}

			// liberate unused map space.
			mapPtr()->liberateStates(idxComp);
		}
		
		
		/** ***************************************************************************************
			MapManager
		******************************************************************************************/
		
		void MapManager::manageDefaultDeletion()
		{
			for(LandmarkList::iterator lmkIter = landmarkList().begin();
					 lmkIter != landmarkList().end(); ++lmkIter)
			{
				landmark_ptr_t lmkPtr = *lmkIter;
				bool needToDie = false;
				for(LandmarkAbstract::ObservationList::iterator obsIter = lmkPtr->observationList().begin();
						obsIter != lmkPtr->observationList().end(); ++obsIter)
				{ // all observations (sensors) must agree to delete a landmark
					observation_ptr_t obsPtr = *obsIter;

					JFR_ASSERT(obsPtr->counters.nMatch <= obsPtr->counters.nSearch, "counters.nMatch " 
										 << obsPtr->counters.nMatch << " > counters.nSearch " << obsPtr->counters.nSearch);
					JFR_ASSERT(obsPtr->counters.nInlier <= obsPtr->counters.nMatch, "counters.nInlier " 
										 << obsPtr->counters.nInlier << " > counters.nMatch " << obsPtr->counters.nMatch);

					// kill if any sensor has search area too large
					if (obsPtr->events.predicted && obsPtr->events.measured && !obsPtr->events.updated)
					{
						if (obsPtr->searchSize > killSizeTh) {
							JFR_DEBUG( "Obs " << lmkPtr->id() << " Killed by size (size " << obsPtr->searchSize << ")" );
							needToDie = true;
							break;
						}
					}
				}
				if (needToDie)
				{
					lmkIter = unregisterLandmark(lmkIter);
				}
			}
		}
		
		void MapManager::manageReparametrization()
		{
			for(LandmarkList::iterator lmkIter = landmarkList().begin();
					 lmkIter != landmarkList().end(); ++lmkIter)
			{
				landmark_ptr_t lmkPtr = *lmkIter;
				if (lmkPtr->converged) continue; // already reparametrized, we don't go back for now
				bool needToReparametrize = true;
				bool hasObserved = false;
				for(LandmarkAbstract::ObservationList::iterator obsIter = lmkPtr->observationList().begin();
						obsIter != lmkPtr->observationList().end(); ++obsIter)
				{ // all observations (sensors) must agree to reparametrize a landmark, and at least one must have observed it
					observation_ptr_t obsPtr = *obsIter;
					if (obsPtr->events.updated) hasObserved = true;
					if (obsPtr->computeLinearityScore() > reparTh)
					{
						needToReparametrize = false;
						break;
					}
				}
				if (hasObserved && needToReparametrize)
					lmkIter = reparametrizeLandmark(lmkIter);
			}
		}

		
		/** ***************************************************************************************
			MapManagerOdometry
		******************************************************************************************/
		
		void MapManagerOdometry::manageDeletion()
		{
			for(MapManagerAbstract::LandmarkList::iterator lmkIter = this->landmarkList().begin();
					 lmkIter != this->landmarkList().end(); ++lmkIter)
			{
				landmark_ptr_t lmkPtr = *lmkIter;
				bool needToDie = true;
				for(LandmarkAbstract::ObservationList::iterator obsIter = lmkPtr->observationList().begin();
						obsIter != lmkPtr->observationList().end(); ++obsIter)
				{ // all observations (sensors) must agree to delete a landmark
					observation_ptr_t obsPtr = *obsIter;

					// kill if all sensors have unstable and inconsistent observations
					if (!(obsPtr->counters.nFrameSinceLastVisible > 0 ||
//					    (obsPtr->counters.nInlier == 1 && !obsPtr->events.updated) ||
					    obsPtr->counters.nSearchSinceLastInlier > 1))
					{
						needToDie = false;
						break;
					}
				}
				if (needToDie)
					lmkIter = unregisterLandmark(lmkIter);
			}
		}
		
		bool MapManagerOdometry::isExclusive(observation_ptr_t obsPtr)
		{
			return (!(obsPtr->counters.nSearchSinceLastInlier > 1));
		}
		
		/** ***************************************************************************************
			MapManagerGlobal
		******************************************************************************************/
		
		void MapManagerGlobal::manageDeletion()
		{
			for(MapManagerAbstract::LandmarkList::iterator lmkIter = this->landmarkList().begin();
					 lmkIter != this->landmarkList().end(); ++lmkIter)
			{
				landmark_ptr_t lmkPtr = *lmkIter;
				bool needToDie = false;
				for(LandmarkAbstract::ObservationList::iterator obsIter = lmkPtr->observationList().begin();
						obsIter != lmkPtr->observationList().end(); ++obsIter)
				{ // all observations (sensors) must agree to delete a landmark
					observation_ptr_t obsPtr = *obsIter;
		
					// kill if all sensors have unstable and inconsistent observations
					if (obsPtr->counters.nSearch > killSearchTh) {
						needToDie = true;
						double matchRatio = obsPtr->counters.nMatch / (double)obsPtr->counters.nSearch;
						double consistencyRatio = obsPtr->counters.nInlier / (double)obsPtr->counters.nMatch;
		
						if (matchRatio >= killMatchTh && consistencyRatio >= killConsistencyTh)
						{
							needToDie = false;
							break;
						}
					}
				}
				if (needToDie)
				{
					JFR_DEBUG( "Obs " << lmkPtr->id() << " Killed by unstability");
					lmkIter = unregisterLandmark(lmkIter);
				}
			}
		}
		
		 bool MapManagerGlobal::mapSpaceForInit()
		{
			if (!MapManager::mapSpaceForInit())
			{
				// TODO delete some landmarks
				return false;
			}
			return true;
		}
		
	}
}

