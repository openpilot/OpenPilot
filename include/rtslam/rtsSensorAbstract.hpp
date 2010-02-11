/*
 * rtsSensorAbstract.h
 *
 *  Created on: Jan 28, 2010
 *      Author: jsola
 */

#ifndef RTSSENSORABSTRACT_H_
#define RTSSENSORABSTRACT_H_

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include "jmath/jblas.hpp"
#include "rtsBlocks.hpp"
#include "rtsObservationAbstract.hpp"
#include <list>
using namespace jblas;
using namespace std;

namespace jafar {

	namespace rtslam {

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all sensors defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsSensorAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~rtsSensorAbstract(void);

				/**
				 * Sensor name
				 */
				string name;

				/**
				 * Sensor type
				 */
				string type;

				/**
				 * Father robot
				 */
				rtsRobotAbstract *robot; //FIXME: check if this is what we want. See if we can avoid ugly pointers and do something neat here.

				/**
				 * pose
				 */
				Pose pose;

				/**
				 * state
				 */
				State state;

				/**
				 * Observations list
				 */
				list<rtsObservationAbstract> observationsList;

				/**
				 * Accessors
				 */
				virtual void setPose(vec pose_);
				virtual void setPoseCov(sym_mat poseCov_);
				virtual vec getPose();
				virtual sym_mat getPoseCov();
				virtual void setState(vec state_);
				virtual void setStateCov(sym_mat stateCov_);
				virtual vec getState();
				virtual sym_mat getStateCov();

				/**
				 * Acquire raw data
				 */
				virtual void acquire();

		};

	}
}

//class Pose {
//	public:
//		indirect_array range;
//		T3D x;
//		sym_mat P;
//};

#endif /* RTSSENSORABSTRACT_H_ */
