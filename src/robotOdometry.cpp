/**
 * robotOdometry.cpp
 *
 *  Created on: 07/03/2010
 *      Author: agonzale
 *
 *  \file robotOdometry.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "boost/numeric/ublas/operation.hpp"
#include "boost/numeric/ublas/matrix_proxy.hpp"
#include "boost/numeric/ublas/vector_proxy.hpp"
#include "rtslam/quatTools.hpp"

#include "rtslam/robotAbstract.hpp"
#include "rtslam/robotOdometry.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;

		/**
		 * Remote constructor from remote map.
		 * \param _map the remote map
		 */
		RobotOdometry::RobotOdometry(const map_ptr_t _mapPtr) :
					RobotAbstract(_mapPtr, RobotOdometry::size(), RobotOdometry::size_control(), RobotOdometry::size_perturbation()) {
					constantPerturbation = false;
					type("Odometry");
				}

		/*
		 * The Jacobian XNEW_x is built with :
		 *   var    |  p       q
		 *      pos |  0       3
		 *   -------+---------------
		 *   p   0  |    PNEW_x
		 *   q   3  |  0     QNEW_q
		 * -------------------------
		 *
		 * The Jacobian XNEW_n is built with :
		 *   var    |  dx       dv
		 *      pos |  0         3
		 *   -------+------------------
		 *   p   0  |PNEW_dx     0
		 *   q   3  |  0     QNEW_dv
		 * ----------------------------
		 */

		void RobotOdometry::move_func(const vec & _x, const vec & _u,
		    const vec & _n, const double _dt, vec & _xnew, mat & _XNEW_x,
		    mat & _XNEW_n) {

			using namespace jblas;
			using namespace ublas;

			//variables
			mat PNEW_x(3,7);
			mat33 PNEW_dx;
			mat34 PNEW_q;
			mat43 QDV_dv, QNEW_dv;
			mat44 QNEW_qdv, QNEW_q;


			// split robot state vector
			vec3 p;
			vec4 q;
			splitState(_x, p, q);

			// split control vector
			// position increment
			// orientation increment
			vec3 dx, dv;
			splitControl(_u, dx, dv);

			// position update
			vec3 pnew;
			vec4 qnew;

			//pnew = quaternion::vecFromFrame(p, dx);

			quaternion::vecFromFrame(_x, dx, pnew, PNEW_x, PNEW_dx);

			//cout<<"pnew : "<<pnew<<endl;

			//quaternion update
			vec4 qdv;


			quaternion::v2q(dv, qdv, QDV_dv);	//vector orientation increment to quaternion with jacobian
			quaternion::qProd(q, qdv, qnew, QNEW_q, QNEW_qdv);

			QNEW_dv = prod(QNEW_qdv, QDV_dv);

			unsplitState(pnew, qnew, _xnew);

			_XNEW_x.clear();
			subrange(_XNEW_x, 3, 7, 3, 7) = QNEW_q;
			subrange(_XNEW_x, 0, 3, 0, 7) = PNEW_x;

			_XNEW_n.clear();
			subrange(_XNEW_n, 0, 3, 0, 3) = PNEW_dx;
			subrange(_XNEW_n, 3, 7, 3, 6) = QNEW_dv;
		}
	}
}
