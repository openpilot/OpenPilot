/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Copyright RIA-LAAS/CNRS, 2010
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * File:      RobotAbstract.h
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

/**
 * \file robotAbstract.hpp
 * File defining the abstract robot class
 * \ingroup rtslam
 */

#ifndef __RobotAbstract_H__
#define __RobotAbstract_H__

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#include <list>

#include <jmath/jblas.hpp>
#include "blocks.hpp"
#include "mapAbstract.hpp"

namespace jafar
{
	namespace rtslam
	{

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all Gaussian control vectors defined in the module rtslam.
		 *
		 * @ingroup rtslam
		 */
		class Control: public Gaussian
		{
				double dt;
			public:
				void set_dt(double dt_)
				{
					dt(dt_);
				}
		};

		/* --------------------------------------------------------------------- */
		/* --- CLASS ----------------------------------------------------------- */
		/* --------------------------------------------------------------------- */

		/** Base class for all robots defined in the module
		 * rtslam.
		 *
		 * @ingroup rtslam
		 */
		class RobotAbstract
		{
			public:

				/**
				 * Mandatory virtual destructor.
				 */
				virtual ~RobotAbstract(void);

				/**
				 * Robot name
				 */
				std::string name;

				/**
				 * Robot type
				 */
				std::string type;

				/**
				 * control structure
				 */
				Control control;

				/**
				 * List of sensors
				 */
				std::list<SensorAbstract*> sensorsList;

				/**
				 * Father map
				 */
				MapAbstract * map;

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
				virtual void acquire(const Control & control_);

				/**
				 * Move the robot.
				 */
				virtual void move(void) = 0;

				/**
				 * Accesssors
				 */
				virtual void setPose(jblas::vec& pose_);
				virtual void setPoseCov(jblas::sym_mat& poseCov_);
				virtual void setPoseCov(jblas::vec& poseStd_);
				virtual jblas::vec getPose();
				virtual jblas::sym_mat getPoseCov();
				virtual void setState(jblas::vec& state_);
				virtual void setStateCov(jblas::sym_mat& stateCov_);
				virtual jblas::vec getState();
				virtual jblas::sym_mat getStateCov();

		};

	}
}

#endif // #ifndef __RobotAbstract_H__
/*
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * tab-width: 2
 * c-basic-offset: 2
 * End:
 */
