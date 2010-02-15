/*
 * robotConstantVelocity.hpp
 *
 *  Created on: 14/02/2010
 *      Author: jsola
 */

/**
 * \file robotConstantVelocity.hpp
 * \ingroup rtslam
 */

#ifndef ROBOTCONSTANTVELOCITY_HPP_
#define ROBOTCONSTANTVELOCITY_HPP_

#include "robotAbstract.hpp"

namespace jafar
{
	namespace rtslam
	{

		/**
		 * Constant velocity model robot class
		 * \ingroup rtslam
		 */
		class Robot3DConstantVelocity: public RobotAbstract
		{
			public:
			void move();

			private:
		};
	}
}

#endif /* ROBOTCONSTANTVELOCITY_HPP_ */
