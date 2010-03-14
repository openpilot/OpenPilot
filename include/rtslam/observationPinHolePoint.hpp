/**
 * observationPinHolePoint.hpp
 *
 *  Created on: 14/03/2010
 *      Author: jsola
 *
 *  \file observationPinHolePoint.hpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef OBSERVATIONPINHOLEPOINT_HPP_
#define OBSERVATIONPINHOLEPOINT_HPP_

#include "rtslam/observationAbstract.hpp"

namespace jafar {
	namespace rtslam {

		/**
		 * Class of observations of points from pin-hole cameras
		 * \author jsola
		 * \ingroup rtslam
		 */
		class ObservationPinHolePoint: public ObservationAbstract {

			public:
				/**
				 * Constructor
				 */
				ObservationPinHolePoint() :
					ObservationAbstract(2) {
				}

				/**
				 * Project an Euclidean point.
				 * \param p a 3D point
				 * \param U_r the Jacobian wrt the robot pose
				 * \param U_s the Jacobian wrt the sensor pose
				 * \param U_p the Jacobian wrt the point
				 */
				void project(const jblas::vec3 & p, jblas::mat & U_r, jblas::mat & U_s, jblas::mat & U_p);

			private:
				// Pin-hole point projection functions
				jblas::vec2 project0(const jblas::vec3 & _v);
				jblas::vec2 distort(const jblas::vec2 & _d, const jblas::vec & _u);
				jblas::vec2 pixellize(const jblas::vec2 & _k, const jblas::vec4 & _ud);
				void project0(const jblas::vec7 & _v, jblas::vec2 _u, jblas::mat U_v);
				void distort(const jblas::vec2 & _d, const jblas::vec & _u, jblas::vec2 & _ud, jblas::mat & UD_u);
				void pixellize(const jblas::vec2 & _k, const jblas::vec4 & _ud, jblas::vec2 & u, jblas::mat & U_ud);

		};

	}
}

#endif /* OBSERVATIONPINHOLEPOINT_HPP_ */
