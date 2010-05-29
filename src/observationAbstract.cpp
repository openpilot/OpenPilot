/**
 * observationAbstract.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file observationAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

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
		std::ostream& operator <<(std::ostream & s, jafar::rtslam::ObservationAbstract & obs) {
			s << "OBSERVATION " << obs.id() << ": of " << obs.landmarkPtr()->type() << " from " << obs.sensorPtr()->type()
			    << endl;
			s << "Sensor: " << obs.sensorPtr()->id() << ", landmark: " << obs.landmarkPtr()->id() << endl;
			s << " .expectation:  " << obs.expectation << endl;
			s << " .measurement:  " << obs.measurement << endl;
			s << " .innovation:   " << obs.innovation << endl;
			s << " .ev: | prj: " << obs.events.predicted << " | vis: " << obs.events.visible << " | mea: " << obs.events.measured << " | mch: " << obs.events.matched << " | upd: " << obs.events.updated << " | ";
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
			categoryName("OBSERVATION");
			clearCounters();
			clearEvents();
		}

		ObservationAbstract::ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr,
		    const size_t _size, const size_t _size_nonobs) :
		    expectation(_size, _size_nonobs),
		    measurement(_size),
		    innovation(_size),
		    prior(_size_nonobs),
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
			id(_lmkPtr->id());
			categoryName("OBSERVATION");
			clearCounters();
			clearEvents();
		}

		void ObservationAbstract::setup(const feature_ptr_t & featPtr, const Gaussian & _prior){
			// todo implement setup()
			measurement.x() = featPtr->state.x();
			measurement.P() = featPtr->state.P();
			prior.x(_prior.x());
			prior.P(_prior.P());
		}


		void ObservationAbstract::project() {

			// Global sensor pose
			vec7 sg;

			// Get global sensor pose
			sensorPtr()->globalPose(sg, SG_rs);

			vec lmk = landmarkPtr()->state.x();
			vec exp, nobs;
			project_func(sg, lmk, exp, nobs, EXP_sg, EXP_l);

			mat EXP_rs = prod(EXP_sg, SG_rs);

			subrange(EXP_rsl, 0,expectation.size(), 0,sensorPtr()->ia_globalPose.size()) = EXP_rs;
			subrange(EXP_rsl, 0,expectation.size(), sensorPtr()->ia_globalPose.size(),sensorPtr()->ia_globalPose.size()+landmarkPtr()->state.size()) = EXP_l;

			expectation.x() = exp;
			expectation.P() = ublasExtra::prod_JPJt(ublas::project(landmarkPtr()->mapPtr()->filterPtr->P(), ia_rsl, ia_rsl), EXP_rsl);

			expectation.nonObs = nobs;

			events.predicted = true;
		}

		void ObservationAbstract::backProject(){
			vec7 sg;

			// Get global sensor pose
			sensorPtr()->globalPose(sg, SG_rs);

			// Make some temporal variables to call function
			vec pix = measurement.x();
			vec invDist = prior.x();
			vec lmk(landmarkPtr()->mySize());
			backProject_func(sg, pix, invDist, lmk, LMK_sg, LMK_meas, LMK_prior);

			landmarkPtr()->state.x(lmk);

			LMK_rs = ublas::prod(LMK_sg, SG_rs);

			// Initialize in map
			landmarkPtr()->mapPtr()->filterPtr->initialize(
					landmarkPtr()->mapPtr()->ia_used_states(),
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
		}

		void ObservationAbstract::predictInfoGain() {
			expectation.infoGain = ublasExtra::det(expectation.P());
		}

		bool ObservationAbstract::compatibilityTest(const double MahaDistSquare){
			return (innovation.mahalanobis() < MahaDistSquare);
		}

		void ObservationAbstract::clearEvents(){
			events.predicted = false;
			events.matched = false;
			events.measured = false;
			events.updated = false;
			events.visible = false;
		}

		void ObservationAbstract::clearCounters(){
			counters.nSearch = 0;
			counters.nMatch = 0;
			counters.nInlier = 0;
		}

		void ObservationAbstract::update() {
			map_ptr_t mapPtr = sensorPtr()->robotPtr()->mapPtr();
			ind_array ia_x = mapPtr->ia_used_states();
			sensorPtr()->robotPtr()->mapPtr()->filterPtr->correct(ia_x,innovation,INN_rsl,ia_rsl) ;
		}

	} // namespace rtslam
} // namespace jafar
