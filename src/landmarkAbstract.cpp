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
	  LandmarkAbstract::LandmarkAbstract(const map_ptr_t & _mapPtr, const landmark_ptr_t & _prevLmk, const size_t _size,jblas::ind_array & _icomp ) :
	    MapObject(_mapPtr,*_prevLmk, _size, _icomp)
		{
			category = LANDMARK;
		}

		LandmarkAbstract::LandmarkAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size) :
			MapObject(_mapPtr, _size, UNFILTERED)
		{
			category = LANDMARK;
		}

		LandmarkAbstract::~LandmarkAbstract() {
//			cout << "Deleted landmark: " << id() << ": " << typeName() << endl;
		}

		bool LandmarkAbstract::needToReparametrize(DecisionMethod repMet){
			switch (repMet) {
				case ANY : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Lazy option: ANY vote for reparametrization declares the need of reparametrization.
						if (obsPtr->voteForReparametrizingLandmark()) return true;
					}
					return false;
				}
				case ALL : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Strict option: ALL votes for reparametrization necessary to declare the need of reparametrization.
						if (!obsPtr->voteForReparametrizingLandmark()) return false;
					}
					return true;
				}
				case MAJORITY : {
					int nRepar = 0, nKeep = 0;
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Democratic option: MAJORITY votes declares the need of reparametrization.
						if (obsPtr->voteForReparametrizingLandmark()) nRepar++;
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



		void LandmarkAbstract::reparametrize(const landmark_ptr_t & lmkPtr) {
//
//			// - create a new STD landmark.
//			if (mapManagerPtr()->mapPtr()->unusedStates(LandmarkEuclideanPoint::size())){
//				cout << __PRETTY_FUNCTION__ << "about to create new lmk" << endl;
//				eucp_ptr_t lmkPtr(new LandmarkEuclideanPoint(mapPtr()));
//
//				// - create its set of observations, one per sensor.
//				observation_ptr_t obsPtr;
//					for (LandmarkAbstract::ObservationList::iterator obsIter = this->observationList().begin(); obsIter != this->observationList().end(); obsIter++)
//					{
//						obsPtr = *obsIter;
//						cout << __PRETTY_FUNCTION__ << "about to create new obs" << endl;
//						obs_ph_euc_ptr_t obsPtrEuc(new ObservationPinHoleEuclideanPoint(obsPtr->sensorPtr(), lmkPtr));
//
//						// - Link the landmark to map and observations.
//						cout << __PRETTY_FUNCTION__ << "about to link new obs to new lmk" << endl;
//						obsPtrEuc->linkToParentEUC(lmkPtr);
//
//						// - Link the sensors to the new observations.
//						cout << __PRETTY_FUNCTION__ << "about to link new obs to sen" << endl;
//						obsPtrEuc->linkToParentSensor(obsPtr->sensorPtr());
//
//						// - transfer info from old obs to new obs.
//						cout << __PRETTY_FUNCTION__ << "about to transfer obs info" << endl;
//						obsPtrEuc->transferInfoObs(obsPtr);
//					}

				// - call reparametrize_func()
					mat LMKNEW_lmk(lmkPtr->mySize(),this->mySize());
					vec lmk = this->state.x();
					vec lmkNEW(lmkPtr->mySize());
					cout << __PRETTY_FUNCTION__ << "about to call reparametrize_func()" << endl;
					reparametrize_func(lmk,lmkNEW,LMKNEW_lmk);
					lmkPtr->state.x(lmkNEW);

				// - call filter->reparametrize()
					cout << __PRETTY_FUNCTION__ << "about to call filter->reparametrize()" << endl;
					mapManagerPtr()->mapPtr()->filterPtr->reparametrize(mapManagerPtr()->mapPtr()->ia_used_states(), LMKNEW_lmk, this->state.ia(), lmkPtr->state.ia());

				// - transfer info from the old lmk to the new one
					landmark_ptr_t lmkPtrOld = this->shared_from_this();
					cout << __PRETTY_FUNCTION__ << "about to transfer lmk info" << endl;
					lmkPtr->transferInfoLmk(lmkPtrOld);

//				// - delete old lmk <-- this will delete all old obs!
//					cout << __PRETTY_FUNCTION__ << "about to kill the old lmk" << endl;
//					this->suicide();
//				}
		}

		void LandmarkAbstract::transferInfoLmk(landmark_ptr_t & lmkSourcePtr){

			this->id(lmkSourcePtr->id());
			this->name(lmkSourcePtr->name());
			this->geomType = lmkSourcePtr->getGeomType();

		}

		bool LandmarkAbstract::needToDie(DecisionMethod dieMet){
			switch (dieMet) {
				case ANY : {
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						// Drastic option: ANY vote for killing declares the need to die.
						if (obsPtr->voteForKillingLandmark()) {
							cout << __PRETTY_FUNCTION__ << ": obs will vote for killing lmk: " << id() << endl;
							return true;
						}
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
//			landmark_ptr_t selfPtr = shared_from_this();
//			mapPtr()->liberateStates(state.ia()); // remove from map
//			cout << __PRETTY_FUNCTION__ << "about to unregister the old lmk" << endl;
//			mapPtr()->ParentOf<LandmarkAbstract>::unregisterChild(selfPtr); // remove from graph
//
//			for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
//			{
//				observation_ptr_t obsPtr = *obsIter;
//				obsIter++;
//				cout << __PRETTY_FUNCTION__ << "about to unregister obs from lmk" << endl;
//				unregisterChild(obsPtr);
//				obsIter--;
//				cout << __PRETTY_FUNCTION__ << "about to unregister obs from sen" << endl;
//				obsPtr->sensorPtr()->ParentOf<ObservationAbstract>::unregisterChild(obsPtr);
//			}
		}

	}
}
