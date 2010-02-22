/**
 * quatTools.hpp
 *
 *  Created on: 20/02/2010
 *      Author: jsola
 *
 * \file quatTools.hpp
 * File defining geometric operations with quaternions.
 * Quaternions operate on vectors and other quaternions.
 * Functions for Jacobian computations are also provided.
 *
 * \ingroup rtslam
 */

#ifndef QUATTOOLS_HPP_
#define QUATTOOLS_HPP_

namespace ublas = boost::numeric::ublas;

namespace jafar {
	namespace rtslam {

		using namespace jblas;
		using namespace jblas;

		/**
		 * Conjugate of quaternion.
		 * \param q The imput quaternion.
		 * \return the conjugated quaternion.
		 */
		template<class VecQ>
		vec4 q2qc(const VecQ & q) {
			vec4 qc;
			qc(0) = +q(0);
			qc(1) = -q(1);
			qc(2) = -q(2);
			qc(3) = -q(3);
			return qc;
		}

		/**
		 * Jacobian of conjugated quaternion wrt input quaternion
		 * Jac = QC_q = dqc/dq.
		 * \param q the input quaternion
		 * \param QC_q the Jacobian.
		 */
		template<class VecQ, class MatQC_q>
		void dqc_by_dq(const VecQ & q, MatQC_q & QC_q) {
			QC_q.clear();
			QC_q(0, 0) = 1.0;
			QC_q(1, 1) = -1.0;
			QC_q(2, 2) = -1.0;
			QC_q(3, 3) = -1.0;
		}

		/**
		 * Conjugate of a quaternion, with Jacobians
		 */
		template<class VecQ, class VecQc, class MatQC_q>
		void q2qc(const VecQ & q, VecQc qc, MatQC_q QC_q) {
			qc = q2qc(q);
			dqc_by_qc(q, QC_q);
		}

		/**
		 * Rotation matrix from quaternion
		 */
		template<class VecQ>
		mat33 q2R(const VecQ & q) {
			double qw = q(0);
			double qx = q(1);
			double qy = q(2);
			double qz = q(3);

			double ww = qw * qw;
			double wx = 2 * qw * qx;
			double wy = 2 * qw * qy;
			double wz = 2 * qw * qz;
			double xx = qx * qx;
			double xy = 2 * qx * qy;
			double xz = 2 * qx * qz;
			double yy = qy * qy;
			double yz = 2 * qy * qz;
			double zz = qz * qz;

			mat33 R;
			R(0, 0) = ww + xx - yy - zz;
			R(0, 1) = xy - wz;
			R(0, 2) = xz + wy;
			R(1, 0) = xy + wz;
			R(1, 1) = ww - xx + yy - zz;
			R(1, 2) = yz - wx;
			R(2, 0) = xz - wy;
			R(2, 1) = yz + wx;
			R(2, 2) = ww - xx - yy + zz;
			return R;
		}

		/**
		 * The transposed rotation matrix from a quaternion
		 * \param q the quaternion.
		 * \return the transposed rotation matrix.
		 */
		template<class VecQ>
		mat33 q2Rt(const VecQ & q) {
			return q2R(q2qc(q));
		}

		/**
		 * Rotate a vector from given quaternion.
		 * \param q the quaternion;
		 * \param v the vector
		 * \return the rotated vector
		 */
		template<class VecQ, class Vec>
		vec3 RofQtimesV(const VecQ & q, const Vec & v) {
			using namespace ublas;
			return prod(q2R(q), v);
		}

		/**
		 * Jacobian of vector rotation from quaternion, wrt quaternion
		 * \param q the quaternion
		 * \param v the vector
		 * \param VO_q the Jacobian of R(q)*v wrt q
		 */
		template<class VecQ, class Vec, class MatVO_q>
		void dRofQtimesV_by_dq(const VecQ & q, const Vec & v, MatVO_q & VO_q) {
			// split q and v
			double qw = q(0);
			double qx = q(1);
			double qy = q(2);
			double qz = q(3);
			double x = v(0);
			double y = v(1);
			double z = v(2);

			// temporary
			double t1 = 2 * (qw * x - qz * y + qy * z);
			double t2 = 2 * (qx * x + qy * y + qz * z);
			double t3 = 2 * (qy * x - qx * y - qw * z);
			double t4 = 2 * (qz * x + qw * y - qx * z);

			// Jacobian
			VO_q(0, 0) = t1;
			VO_q(0, 1) = t2;
			VO_q(0, 2) = -t3;
			VO_q(0, 3) = -t4;
			VO_q(1, 0) = t4;
			VO_q(1, 1) = t3;
			VO_q(1, 2) = t2;
			VO_q(1, 3) = t1;
			VO_q(2, 0) = -t3;
			VO_q(2, 1) = t4;
			VO_q(2, 2) = -t1;
			VO_q(2, 3) = t2;
		}

		/**
		 * Jacobian of vector rotation from quaternion, wrt vector
		 * \param q the quaternion
		 * \param VO_v = R(q) the Jacobian of R(q)*v wrt v
		 */
		template<class VecQ, class MatVO_v>
		void dRofQtimesV_by_dv(const VecQ & q, MatVO_v & VO_v) {
			VO_v = q2R(q);
		}

		/**
		 * Rotate a vector from given quaternion, with all Jacobians.
		 * \param q the quaternion;
		 * \param v the vector
		 * \param vo the rotated vector
		 * \param VO_q the Jacobian wrt q
		 * \param VO_v the Jacobians wrt v
		 */
		template<class VecQ, class Vec, class VecO, class MatVO_q, class MatVO_v>
		void RofQtimesV(const VecQ & q, const Vec & v, VecO & vo, MatVO_q & VO_q, MatVO_v VO_v) {
				vo = RofQtimesV(q,v);
				dRofQtimesV_by_dq(q, v, VO_q);
				dRofQtimesV_by_dv(q, VO_v);
		}

	}
}

#endif /* QUATTOOLS_HPP_ */
