/**
 * landmarkAbstract.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file landmarkAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/observationAbstract.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationPinHoleEuclideanPoint.hpp"
#include "rtslam/ahpTools.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;

		IdFactory LandmarkAbstract::landmarkIds = IdFactory();

		/*
		 * constructor.
		 */
		LandmarkAbstract::LandmarkAbstract(const map_ptr_t & _mapPtr, const size_t _size) :
			MapObject(_mapPtr, _size)
		{
			category = LANDMARK;
		}

		LandmarkAbstract::LandmarkAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size) :
			MapObject(_mapPtr, _size, UNFILTERED)
		{
			category = LANDMARK;
		}

		LandmarkAbstract::~LandmarkAbstract() {
			for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
			{
				observation_ptr_t obsPtr = *obsIter;
				obsPtr->sensorPtr()->ParentOf<ObservationAbstract>::unregisterChild(obsPtr);
//				cout << "Should unregister obs: " << obsPtr->id() << endl;
			}
//			cout << "Deleted landmark: " << id() << endl;
		}

		bool LandmarkAbstract::needToReparametrize(DecisionMethod repMet){
			switch (repMet) {
				case ANY : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Drastic option: ANY vote for reparametrization declares the need of reparametrization.
						if (obsPtr->voteForReparametrizeLandmark()) return true;
					}
					return false;
				}
				case ALL : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Magnanimous option: ALL votes for reparametrization necessary to declare the need of reparametrization.
						if (!obsPtr->voteForReparametrizeLandmark()) return false;
					}
					return true;
				}
				case MAJORITY : {
					int nRepar = 0, nKeep = 0;
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Democratic option: MAJORITY votes declares the need of reparametrization.
						if (obsPtr->voteForReparametrizeLandmark()) nRepar++;
						else nKeep++;
					}
					if (nRepar > nKeep) return true;
					return false;

				}
				default : {
					cout << __FILE__ << ":" << __LINE__ << ": Bad evaluation method. Using ANY." << endl;
					return needToReparametrize(ANY);
				}
			}
			return false;
		}



		void LandmarkAbstract::reparametrize() {
			//TODO Implement reparametrize():

			// - create a new STD landmark.
			if (mapPtr()->unusedStates(LandmarkEuclideanPoint::size())){
				eucp_ptr_t lmkPtr(new LandmarkEuclideanPoint(mapPtr()));

				// - create its set of observations, one per sensor.
				observation_ptr_t obsPtr;
					for (LandmarkAbstract::ObservationList::iterator obsIter = this->observationList().begin(); obsIter != this->observationList().end(); obsIter++)
					{
						obsPtr = *obsIter;
						obs_ph_euc_ptr_t obsPtrEuc(new ObservationPinHoleEuclideanPoint(obsPtr->sensorPtr(), lmkPtr));

						// - Link the landmark to map and observations.
						obsPtrEuc->linkToParentEUC(lmkPtr);

						// - Link the sensors to the new observations.
						obsPtrEuc->linkToParentSensor(obsPtr->sensorPtr());

						// - transfer info from old obs to new obs.
						obsPtrEuc->transferInfoObs(obsPtr);

					}

				// - call reparametrize_func()
					mat LMKNEW_lmk(lmkPtr->mySize(),this->mySize());
					vec lmk = this->state.x();
					vec lmkNEW(lmkPtr->mySize());
					reparametrize_func(lmk,lmkNEW,LMKNEW_lmk);
					lmkPtr->state.x(lmkNEW);

				// - call filter->reparametrize()
					mapPtr()->filterPtr->reparametrize(mapPtr()->ia_used_states(),LMKNEW_lmk,this->state.ia(),lmkPtr->state.ia());

				// - transfer info from the old lmk to the new one
					lmkPtr->transferInfoLmk(this->shared_from_this());

				// - delete old lmk <-- this will delete all old obs!
					this->suicide();
				}
		}

		bool LandmarkAbstract::needToDie(DecisionMethod dieMet){
			switch (dieMet) {
				case ANY : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Drastic option: ANY vote for killing declares the need to die.
						if (obsPtr->voteForKillingLandmark()) return true;
					}
					return false;
				}
				case ALL : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Magnanimous option: ALL votes for killing necessary to declare the need to die.
						if (!obsPtr->voteForKillingLandmark()) return false;
					}
					return true;
				}
				case MAJORITY : {
					int nDie = 0, nSurvive = 0;
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Democratic option: MAJORITY votes declares the need to die.
						if (obsPtr->voteForKillingLandmark()) nDie++;
						else nSurvive++;
					}
					if (nDie > nSurvive) return true;
					return false;

				}
				default : {
					cout << __FILE__ << ":" << __LINE__ << ": Bad evaluation method. Using ANY." << endl;
					return needToDie(ANY);
				}
			}
			return false;
		}

		void LandmarkAbstract::suicide(){
			landmark_ptr_t selfPtr = shared_from_this();
			mapPtr()->liberateStates(state.ia()); // remove from map
			mapPtr()->ParentOf<LandmarkAbstract>::unregisterChild(selfPtr); // remove from graph
		}

	}
}
