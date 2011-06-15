/**
 * \file landmarkAbstract.cpp
 * \date 10/03/2010
 * \author jsola
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

		std::ostream& operator <<(std::ostream & s, LandmarkAbstract const & lmk) {
			s << "LANDMARK " << lmk.id() << ": of " << lmk.typeName() << endl;
			s << " .state:  " << lmk.state << endl;
         DescriptorAbstract *desc = lmk.descriptorPtr.get();
         if(desc != NULL)
            s << " .descriptor:" << *desc << std::endl;
         else
            s << " No descriptor " << std::endl;
         return s;
		}
		
		image::oimstream& operator <<(image::oimstream & s, LandmarkAbstract const & lmk) {
			DescriptorAbstract *desc = lmk.descriptorPtr.get();
			s << *desc;
			return s;
		}
		
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
			descriptorPtr = _prevLmk->descriptorPtr;
			visibilityMap = _prevLmk->visibilityMap;
		}

		LandmarkAbstract::LandmarkAbstract(const simulation_t dummy, const map_ptr_t & _mapPtr, const size_t _size) :
			MapObject(_mapPtr, _size, UNFILTERED)
		{
			category = LANDMARK;
		}

		LandmarkAbstract::~LandmarkAbstract() {
//			cout << "Deleted landmark: " << id() << ": " << typeName() << endl;
		}

#if 0
		bool LandmarkAbstract::needToDie(DecisionMethod dieMet){
			switch (dieMet) {
				case ANY : {
					// Drastic option: ANY vote for killing declares the need to die.
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						if (obsPtr->voteForKillingLandmark()) {
							return true;
						}
					}
					return false;
				}
				case ALL : {
					// Magnanimous option: ALL votes for killing are necessary to declare the need to die.
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						if (!obsPtr->voteForKillingLandmark()) return false;
					}
					return true;
				}
				case MAJORITY : {
					// Democratic option: MAJORITY of votes determines the need to die.
					unsigned int nDie = 0, nSurvive = 0;
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
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

		
		bool LandmarkAbstract::needToReparametrize(DecisionMethod repMet){
			if (converged) return false;
			// first check if the landmark was observed this frame, always with the ANY policy
			bool observed = false;
			for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
			{
				observation_ptr_t obsPtr = *obsIter;
				if (obsPtr->events.updated) { observed = true; break; }
			}
			if (!observed) return false;
			
			// now check according to the chosen policy that the linearity scores are ok
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
					cout << __FILE__ << ":" << __LINE__ << ": Bad evaluation method. Using ALL." << endl;
					return needToReparametrize(ALL);
				}
			}
			return false;
		}
#endif

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
//					cout << __PRETTY_FUNCTION__ << "about to call reparametrize_func()" << endl;
					reparametrize_func(lmk,lmkNEW,LMKNEW_lmk);
					lmkPtr->state.x(lmkNEW);

				// - call filter->reparametrize()
//					cout << __PRETTY_FUNCTION__ << "about to call filter->reparametrize()" << endl;
					mapManagerPtr()->mapPtr()->filterPtr->reparametrize(mapManagerPtr()->mapPtr()->ia_used_states(), LMKNEW_lmk, this->state.ia(), lmkPtr->state.ia());

				// - transfer info from the old lmk to the new one
					landmark_ptr_t lmkPtrOld = this->shared_from_this();
//					cout << __PRETTY_FUNCTION__ << "about to transfer lmk info" << endl;
					lmkPtr->transferInfoLmk(lmkPtrOld);

//				// - delete old lmk <-- this will delete all old obs!
//					cout << __PRETTY_FUNCTION__ << "about to kill the old lmk" << endl;
//					this->suicide();
//				}
		}

		void LandmarkAbstract::reparametrize(int size, vec &xNew, sym_mat &pNew) {
			mat XNEW_xold(size,this->mySize());
			xNew.resize(size, false);
			vec xOld = this->state.x();
			sym_mat pOld = sym_adapt(this->state.P()); // sym_mat should not be necessary as P is sym, but it sometimes fails at typecheck...
			reparametrize_func(xOld,xNew,XNEW_xold);
			mat tmp = ublas::prod(XNEW_xold, pOld);
			pNew = ublas::prod(tmp, ublas::trans(XNEW_xold));
		}


		void LandmarkAbstract::transferInfoLmk(landmark_ptr_t & lmkSourcePtr){

			this->id(lmkSourcePtr->id());
			this->name(lmkSourcePtr->name());
			this->geomType = lmkSourcePtr->getGeomType();

		}
#if 0
		bool LandmarkAbstract::needToDie(DecisionMethod dieMet){
			switch (dieMet) {
				case ANY : {
					// Drastic option: ANY vote for killing declares the need to die.
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						if (obsPtr->voteForKillingLandmark()) {
							return true;
						}
					}
					return false;
				}
				case ALL : {
					// Magnanimous option: ALL votes for killing are necessary to declare the need to die.
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
						if (!obsPtr->voteForKillingLandmark()) return false;
					}
					return true;
				}
				case MAJORITY : {
					// Democratic option: MAJORITY of votes determines the need to die.
					unsigned int nDie = 0, nSurvive = 0;
					for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
					{
						observation_ptr_t obsPtr = *obsIter;
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
#endif
		void LandmarkAbstract::destroyDisplay()
		{
			ObjectAbstract::destroyDisplay();
			for (ObservationList::iterator obsIter = observationList().begin(); obsIter != observationList().end(); obsIter++)
				(*obsIter)->destroyDisplay();
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
