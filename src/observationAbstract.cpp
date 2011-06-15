/**
 * \file observationAbstract.cpp
 * \date 10/03/2010
 * \author jsola
 * \ingroup rtslam
 */

#include "kernel/jafarDebug.hpp"

#include "rtslam/observationAbstract.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

#include "rtslam/featureAbstract.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		//////////////////////////
		// OBSERVATION ABSTRACT
		//////////////////////////

		/*
		 * Operator << for class ObservationAbstract.
		 * It shows different information of the observation.
		 */
		std::ostream& operator <<(std::ostream & s, ObservationAbstract const & obs) {
			s << "OBSERVATION " << obs.id() << ": of " << obs.typeName() << " from landmark " << obs.landmarkPtr()->typeName() << " and sensor " << obs.sensorPtr()->typeName()
			    << endl;
			s << "Sensor: " << obs.sensorPtr()->id() << ", landmark: " << obs.landmarkPtr()->id() << endl;
			s << " .expectation:  " << obs.expectation << endl;
			s << " .measurement:  " << obs.measurement << endl;
			s << " .innovation:   " << obs.innovation << endl;
			s << " .ev: | prj: " << obs.events.predicted
					<< " | vis: " << obs.events.visible
					<< " | mea: " << obs.events.measured
					<< " | mch: " << obs.events.matched
					<< " | upd: " << obs.events.updated << " | " << endl;
			s << " .cnt:| search: " << obs.counters.nSearch
					<< " | match: " << obs.counters.nMatch
					<< " | inlier: " << obs.counters.nInlier << " | ";
			s << endl << " .visibility: " << obs.landmark().visibilityMap; // FIXME with different sensors
			return s;
		}
		
		image::oimstream& operator <<(image::oimstream & s, ObservationAbstract const & obs) {
			obs.desc_image(s);
			return s;
		}

		ObservationAbstract::ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr,
		    const size_t _size_meas, const size_t _size_exp, const size_t _size_inn, const size_t _size_nonobs) :
		    expectation(_size_exp, _size_nonobs),
		    measurement(_size_meas),
		    innovation(_size_inn),
		    prior(_size_nonobs),
		    ia_rsl(ublasExtra::ia_union(_senPtr->ia_globalPose, _lmkPtr->state.ia())),
		    EXP_sg(_size_exp, 7),
		    EXP_l(_size_exp, _lmkPtr->state.size()),
		    EXP_rsl(_size_exp, ia_rsl.size()),
		    INN_meas(_size_inn, _size_meas),
		    INN_exp(_size_inn, _size_exp),
		    INN_rsl(_size_inn, ia_rsl.size()),
		    LMK_sg(_lmkPtr->state.size(),7),
		    LMK_meas(_lmkPtr->state.size(),_size_meas),
		    LMK_prior(_lmkPtr->state.size(),_size_nonobs),
		    LMK_rs(_lmkPtr->state.size(),_senPtr->ia_globalPose.size())
		{
			clearCounters();
			clearFlags();
			searchSize = 0;
		}

		ObservationAbstract::ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr,
		    const size_t _size, const size_t _size_nonobs) :
		    expectation(_size, _size_nonobs),
		    measurement(_size),
		    innovation(_size),
		    prior(_size_nonobs),
		    noiseCovariance(_size),
		    ia_rsl(ublasExtra::ia_union(_senPtr->ia_globalPose, _lmkPtr->state.ia())),
		    SG_rs(7, _senPtr->ia_globalPose.size()),
		    EXP_sg(_size, 7),
		    EXP_l(_size, _lmkPtr->state.size()),
		    EXP_rsl(_size, ia_rsl.size()),
		    INN_meas(_size, _size),
		    INN_exp(_size, _size),
		    INN_rsl(_size, ia_rsl.size()),
		    LMK_sg(_lmkPtr->state.size(),7),
		    LMK_meas(_lmkPtr->state.size(),_size),
		    LMK_prior(_lmkPtr->state.size(),_size_nonobs),
		    LMK_rs(_lmkPtr->state.size(),_senPtr->ia_globalPose.size())
		{
	    category = OBSERVATION;
			id(_lmkPtr->id());
			clearCounters();
			clearFlags();
			searchSize = 0;
		}

		ObservationAbstract::~ObservationAbstract() {
//			cout << "Deleted observation: " << id() << ": " << typeName() << endl;
		}


////		void ObservationAbstract::setup(const feature_ptr_t & featPtr, const Gaussian & _prior){
//		void ObservationAbstract::setup(double _noiseStd, const Gaussian & _prior){
//			noiseCovariance.clear();
//			_noiseStd *= _noiseStd;
//			for (size_t i = 0; i < measurement.size(); i++){
//				noiseCovariance(i,i) = _noiseStd;
//			}
//			measurement.clear();
//			measurement.P(noiseCovariance);
//			prior.x(_prior.x());
//			prior.P(_prior.P());
//		}

		//		void ObservationAbstract::setup(const feature_ptr_t & featPtr, const Gaussian & _prior){
				void ObservationAbstract::setPrior(const Gaussian & _prior) {
					prior = _prior;
//					prior.x(_prior.x());
//					prior.P(_prior.P());
				}

		void ObservationAbstract::project() {
			// Get global sensor pose
			vec7 sg;
			sensorPtr()->globalPose(sg, SG_rs);

			// project lmk
			vec lmk = landmarkPtr()->state.x();
			vec exp, nobs;
			model->project_func(sg, lmk, exp, nobs, EXP_sg, EXP_l);

			// chain rule for Jacobians
			mat EXP_rs = prod(EXP_sg, SG_rs);
			subrange(EXP_rsl, 0, expectation.size(), 0, sensorPtr()->ia_globalPose.size()) = EXP_rs;
			subrange(EXP_rsl, 0, expectation.size(), sensorPtr()->ia_globalPose.size(), sensorPtr()->ia_globalPose.size()+landmarkPtr()->state.size()) = EXP_l;

			// Assignments:
			// x+ = f(x, u, n) :
			expectation.x() = exp;
			// P+ = F_x * P * F_x' + F_n * Q * F_n' :
         expectation.P() = ublasExtra::prod_JPJt(ublas::project(landmarkPtr()->mapManagerPtr()->mapPtr()->filterPtr->P(), ia_rsl, ia_rsl), EXP_rsl);
//         JFR_DEBUG("EXP_rsl \n" << EXP_rsl);
//         JFR_DEBUG("ia_rsl \n" << ia_rsl);
//         JFR_DEBUG("proj \n" << ublas::project(landmarkPtr()->mapManagerPtr()->mapPtr()->filterPtr->P(), ia_rsl, ia_rsl));
//         JFR_DEBUG("expectation \n" << expectation.x() << "\n" << expectation.P());
			// non-observable
			expectation.nonObs = nobs;

			// Events
			events.predicted = true;
// JFR_DEBUG("projected obs " << id() << ": expectation " << expectation.x() << " " << expectation.P());
		}
		
		void ObservationAbstract::projectMean() {
			vec7 sg = sensorPtr()->globalPose();

			vec lmk = landmarkPtr()->state.x();
			vec exp, nobs;
			model->project_func(sg, lmk, exp, nobs);

			expectation.x() = exp;
			expectation.nonObs = nobs;
		}

		void ObservationAbstract::backProject(){
			vec7 sg;

			// Get global sensor pose
			sensorPtr()->globalPose(sg, SG_rs);

			// Make some temporal variables to call function
			vec pix = measurement.x();
			vec invDist = prior.x();
			vec lmk(landmarkPtr()->mySize());
			model->backProject_func(sg, pix, invDist, lmk, LMK_sg, LMK_meas, LMK_prior);

			landmarkPtr()->state.x(lmk);

			LMK_rs = ublas::prod(LMK_sg, SG_rs);

			// Initialize in map
			landmarkPtr()->mapManagerPtr()->mapPtr()->filterPtr
			  ->initialize(
				       landmarkPtr()->mapManagerPtr()->mapPtr()->ia_used_states(),
					LMK_rs,
					sensorPtr()->ia_globalPose,
					landmarkPtr()->state.ia(),
					LMK_meas,
					measurement.P(),
					LMK_prior,
					prior.P());
		}

		void ObservationAbstract::computeInnovation() {
			innovation.x() = measurement.x() - expectation.x();
			innovation.P() = measurement.P() + expectation.P();
			INN_rsl = -EXP_rsl;
		}
		
		void ObservationAbstract::computeInnovationMean(vec &inn, const vec &meas, const vec &exp) const
		{
			inn = meas - exp;
		}

		double ObservationAbstract::computeRelevance() {
			// compute innovation score = squared mahalanobis distance of innovation.x wrt measurement.P
			// but for faster result, and because measurement.P will probably always be diagonal, only use diagonal elements
			innovation.relevance = 0.0;
			for (size_t i = 0; i < measurement.size(); ++i)
				innovation.relevance += jmath::sqr(innovation.x()(i))/measurement.P(i,i);
			return innovation.relevance;
		}

		void ObservationAbstract::predictInfoGain() {
			expectation.infoGain = ublasExtra::det(expectation.P());
		}

		bool ObservationAbstract::compatibilityTest(double mahaDist){
//JFR_DEBUG("obs " << id() << " passing compatibilityTest with " << mahaDist);
			return (innovation.mahalanobis() < mahaDist*mahaDist);
		}

		void ObservationAbstract::clearFlags(){
			int size = sizeof(Events)/sizeof(bool);
			for (int i = 0; i < size; ++i)
				((bool*)&events)[i] = false;
			size = sizeof(Tasks)/sizeof(bool);
			for (int i = 0; i < size; ++i)
				((bool*)&tasks)[i] = false;
		}

		void ObservationAbstract::clearCounters(){
			int size = sizeof(Counters)/sizeof(int);
			for (int i = 0; i < size; ++i)
				((int*)&counters)[i] = 0;
		}

		void ObservationAbstract::update() {
			map_ptr_t mapPtr = sensorPtr()->robotPtr()->mapPtr();
			ind_array ia_x = mapPtr->ia_used_states();
			mapPtr->filterPtr->correct(ia_x,innovation,INN_rsl,ia_rsl) ;
		}
#if 0
		bool ObservationAbstract::voteForKillingLandmark(){
			// kill big ellipses
			// FIXME this is ok for 1 sensor, but not for more, because it won't work with policy ALL
			// probably wee need to fix the policy, and chose a different policy according to the criteria
			// (size = ANY, matchRatio = ALL, consistencyRatio = ...)
			if (events.predicted && events.measured && !events.updated)
			{
				if (searchSize > killSizeTh) {
					JFR_DEBUG( "Obs " << id() << " Killed by size (size " << searchSize << ")" );
					return true;
				}
			}

			// kill unstable and inconsistent lmks
			JFR_ASSERT(counters.nMatch <= counters.nSearch, "counters.nMatch " << counters.nMatch << " > counters.nSearch " << counters.nSearch);
			JFR_ASSERT(counters.nInlier <= counters.nMatch, "counters.nInlier " << counters.nInlier << " > counters.nMatch " << counters.nMatch);
			if (counters.nSearch > killSearchTh) {
				double matchRatio = counters.nMatch / (double) counters.nSearch;
				double consistencyRatio = counters.nInlier / (double)counters.nMatch;

				if (matchRatio < killMatchTh || consistencyRatio < killConsistencyTh)	{
					JFR_DEBUG( "Obs " << id() << " Killed by unstability (match " << matchRatio << " ; consistency " << consistencyRatio << ")" );
					return true;
				}
			}
			return false;
		}
#endif
		void ObservationAbstract::transferInfoObs(observation_ptr_t & obs){
			this->id(obs->id());
			this->name(obs->name());

			this->expectation.x(obs->expectation.x());
			this->expectation.P(obs->expectation.P());
			this->expectation.infoGain = obs->expectation.infoGain;
			this->expectation.nonObs = obs->expectation.nonObs;
			this->expectation.visible = obs->expectation.visible;

			this->innovation.x(obs->innovation.x());
			this->innovation.P(obs->innovation.P());
			this->innovation.iP_ = obs->innovation.iP_;
			this->innovation.mahalanobis_ = obs->innovation.mahalanobis_;

			this->measurement.x(obs->measurement.x());
			this->measurement.P(obs->measurement.P());
			this->measurement.matchScore = obs->measurement.matchScore;

			this->counters = obs->counters;
			this->events = obs->events;
			this->searchSize = obs->searchSize;
		}

	} // namespace rtslam
} // namespace jafar
