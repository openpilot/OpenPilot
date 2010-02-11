/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      rtsRobotAbstract.h
 * Project:   RT-Slam
 * Author:    Joan Sola
 *
 * Version control
 * ===============
 *
 *  $Id$
 *
 * Description
 * ============
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef __rtsRobotAbstract_H__
#define __rtsRobotAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <jmath/jblas.hpp>
#include <list>
#include "rtsBlocks.hpp"
using namespace jblas;
using namespace std;

namespace jafar {

	namespace rtslam {

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class rtsRobotAbstract {
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~rtsRobotAbstract(void);

				/**
				 * Robot name
				 */
				string name;

				/**
				 * Robot type
				 */
				string type;

				/**
				 * control structure
				 */
				Control control;

				/**
				 * List of sensors
				 */
				list<rtsSensorAbstract> sensorsList;

				/**
				 * pose structure
				 */
				Pose pose;

				/**
				 * state structure
				 */
				State state;

				/**
				 * Acquire control structure
				 */
				virtual void acquire();

				/**
				 * Move the robot.
				 */
				virtual void move(void) = 0;

				/**
				 * Accesssors
				 */
				virtual void setPose(vec pose_);
				virtual void setPoseCov(mat poseCov_);
				virtual vec getPose();
				virtual sym_mat getPoseCov();
				virtual void setState(vec state_);
				virtual void setStateCov(sym_mat stateCov_);
				virtual vec getState();
				virtual sym_mat getStateCov();

		};

	}
}

#endif // #ifndef __rtsRobotAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
