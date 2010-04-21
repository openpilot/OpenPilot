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
			s << "OBSERVATION " << obs.id() << ": of " << obs.landmarkPtr->type() << " from " << obs.sensorPtr->type()
			    << endl;
			s << "Sensor: " << obs.sensorPtr->id() << ", landmark: " << obs.landmarkPtr->id() << endl;
			s << " .expectation:  " << obs.expectation << endl;
			s << " .measurement:  " << obs.measurement << endl;
			s << " .innovation:   " << obs.innovation;
			return s;
		}

		ObservationAbstract::ObservationAbstract(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr,
		    const size_t _size_meas, const size_t _size_exp, const size_t _size_inn, const size_t _size_nonobs) :
		    sensorPtr(_senPtr),
		    landmarkPtr(_lmkPtr),
		    expectation(_size_exp, _size_nonobs),
		    measurement(_size_meas),
		    innovation(_size_inn),
		    prior(_size_nonobs),
		    ia_rsl(ublasExtra::ia_union(sensorPtr->ia_globalPose, landmarkPtr->state.ia())),
		    EXP_sg(_size_exp, 7),
		    EXP_l(_size_exp, _lmkPtr->state.size()),
		    EXP_rsl(_size_exp, ia_rsl.size()),
		    INN_meas(_size_inn, _size_meas),
		    INN_exp(_size_inn, _size_exp),
		    INN_rsl(_size_inn, ia_rsl.size()),
		    LMK_sg(_lmkPtr->state.size(),7),
		    LMK_meas(_lmkPtr->state.size(),_size_meas),
		    LMK_prior(_lmkPtr->state.size(),_size_nonobs),
		    LMK_rs(_lmkPtr->state.size(),sensorPtr->ia_globalPose.size())
		{
			categoryName("OBSERVATION");
		}

		void ObservationAbstract::link(const sensor_ptr_t & _senPtr, const landmark_ptr_t & _lmkPtr) {
			sensorPtr = _senPtr;
			landmarkPtr = _lmkPtr;
			ia_rsl = ublasExtra::ia_union(sensorPtr->ia_globalPose, landmarkPtr->state.ia());
		}

		void ObservationAbstract::project() {

			// Global sensor pose
			vec7 sg;

			// Get global sensor pose
			sensorPtr->globalPose(sg, SG_rs);

			vec lmk = landmarkPtr->state.x();
			vec exp, nobs;
			project_func(sg, lmk, exp, nobs, EXP_sg, EXP_l);

			expectation.x() = exp;
			expectation.P() = ublasExtra::prod_JPJt(ublas::project(landmarkPtr->mapPtr->filter.P(), ia_rsl, ia_rsl), EXP_rsl);
		}

		void ObservationAbstract::backProject(){
			vec7 sg;

			// Get global sensor pose
			sensorPtr->globalPose(sg, SG_rs);

			// Make some temporal variables to call function
			vec pix = measurement.x();
			vec invDist = prior.x();
			vec lmk = landmarkPtr->state.x();
			backProject_func(sg, pix, invDist, lmk, LMK_sg, LMK_meas, LMK_prior);

			LMK_rs = ublas::prod(LMK_sg, SG_rs);

			// Initialize in map
			landmarkPtr->mapPtr->filter.initialize(
					landmarkPtr->mapPtr->ia_used_states(),
					LMK_rs,
					sensorPtr->ia_globalPose,
					landmarkPtr->state.ia(),
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
			events.matched = false;
			events.measured = false;
			events.updated = false;
			events.visible = false;
		}

	} // namespace rtslam
} // namespace jafar
