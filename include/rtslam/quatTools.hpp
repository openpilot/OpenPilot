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
			vo = RofQtimesV(q, v);
			dRofQtimesV_by_dq(q, v, VO_q);
			dRofQtimesV_by_dv(q, VO_v);
		}

		/**
		 * Rotate inversely a vector from given quaternion.
		 * \param q the quaternion;
		 * \param v the vector
		 * \return the rotated vector
		 */
		template<class VecQ, class Vec>
		vec3 RTofQtimesV(const VecQ & q, const Vec & v) {
			using namespace ublas;
			return prod(q2Rt(q), v);
		}

		/**
		 * Jacobian of inverse vector rotation from quaternion, wrt quaternion
		 * \param q the quaternion
		 * \param v the vector
		 * \param VO_q the Jacobian of R(q)'*v wrt q
		 */
		template<class VecQ, class Vec, class MatVO_q>
		void dRTofQtimesV_by_dq(const VecQ & q, const Vec & v, MatVO_q & VO_q) {
			// split q and v
			double qw = q(0);
			double qx = q(1);
			double qy = q(2);
			double qz = q(3);
			double x = v(0);
			double y = v(1);
			double z = v(2);

			// temporary
			double s1 = 2 * (qw * x + qz * y - qy * z);
			double s2 = 2 * (qx * x + qy * y + qz * z);
			double s3 = 2 * (qy * x - qx * y + qw * z);
			double s4 = 2 * (qz * x - qw * y - qx * z);

			// Jacobian
			VO_q(0, 0) = s1;
			VO_q(0, 1) = s2;
			VO_q(0, 2) = -s3;
			VO_q(0, 3) = -s4;
			VO_q(1, 0) = -s4;
			VO_q(1, 1) = s3;
			VO_q(1, 2) = s2;
			VO_q(1, 3) = -s1;
			VO_q(2, 0) = s3;
			VO_q(2, 1) = s4;
			VO_q(2, 2) = s1;
			VO_q(2, 3) = s2;
		}

		/**
		 * Jacobian of vector inverse rotation from quaternion, wrt vector
		 * \param q the quaternion
		 * \param VO_v = R(q) the Jacobian of R(q)'*v wrt v
		 */
		template<class VecQ, class MatVO_v>
		void dRTofQtimesV_by_dv(const VecQ & q, MatVO_v & VO_v) {
			VO_v = q2Rt(q);
		}

		/**
		 * Rotate inversely a vector from given quaternion, with all Jacobians.
		 * \param q the quaternion;
		 * \param v the vector
		 * \param vo the rotated vector
		 * \param VO_q the Jacobian wrt q
		 * \param VO_v the Jacobians wrt v
		 */
		template<class VecQ, class Vec, class VecO, class MatVO_q, class MatVO_v>
		void RTofQtimesV(const VecQ & q, const Vec & v, VecO & vo, MatVO_q & VO_q, MatVO_v VO_v) {
			VO_v = q2Rt(q); // this is Rt !
			vo = prod(VO_v, v);
			dRTofQtimesV_by_dq(q, v, VO_q);
		}

		/**
		 * Quaternion product
		 */
		template<class VecQ1, class VecQ2>
		vec4 qProd(const VecQ1 q1, const VecQ2 q2) {
			// split quaternions
			double q1w = q1(0);
			double q1x = q1(1);
			double q1y = q1(2);
			double q1z = q1(3);
			double q2w = q2(0);
			double q2x = q2(1);
			double q2y = q2(2);
			double q2z = q2(3);

			vec4 q;
			q(0) = q1w * q2w - q1x * q2x - q1y * q2y - q1z * q2z;
			q(1) = q1w * q2x + q1x * q2w + q1y * q2z - q1z * q2y;
			q(2) = q1w * q2y - q1x * q2z + q1y * q2w + q1z * q2x;
			q(3) = q1w * q2z + q1x * q2y - q1y * q2x + q1z * q2w;
			return q;
		}

		/**
		 * Jacobian of quaternion product wrt first quaternion, d(q1*q2)/dq1
		 * This Jacobian only depends on q2
		 * \param q2 the second quaternion of the product
		 * \return the Jacobian
		 */
		template<class VecQ2, class MatQ_q1>
		void dQProd_by_dq1(const VecQ2 q2, MatQ_q1 Q_q1) {

			// split quaternion
			double q2w = q2(0);
			double q2x = q2(1);
			double q2y = q2(2);
			double q2z = q2(3);

			Q_q1(0, 0) = q2w;
			Q_q1(0, 1) = -q2x;
			Q_q1(0, 2) = -q2y;
			Q_q1(0, 3) = -q2z;
			Q_q1(1, 0) = q2x;
			Q_q1(1, 1) = q2w;
			Q_q1(1, 2) = q2z;
			Q_q1(1, 3) = -q2y;
			Q_q1(2, 0) = q2y;
			Q_q1(2, 1) = -q2z;
			Q_q1(2, 2) = q2w;
			Q_q1(2, 3) = q2x;
			Q_q1(3, 0) = q2z;
			Q_q1(3, 1) = q2y;
			Q_q1(3, 2) = -q2x;
			Q_q1(3, 3) = q2w;
		}

		/**
		 * Jacobian of quaternion product wrt second quaternion, d(q1*q2)/dq2
		 * This Jacobian only depends on q1
		 * \param q1 the first quaternion of the product
		 * \return the Jacobian
		 */
		template<class VecQ1, class MatQ_q2>
		void dQProd_by_dq2(const VecQ1 q1, MatQ_q2 Q_q2) {

			// split quaternion
			double q1w = q1(0);
			double q1x = q1(1);
			double q1y = q1(2);
			double q1z = q1(3);

			Q_q2(0, 0) = q1w;
			Q_q2(0, 1) = -q1x;
			Q_q2(0, 2) = -q1y;
			Q_q2(0, 3) = -q1z;
			Q_q2(1, 0) = q1x;
			Q_q2(1, 1) = q1w;
			Q_q2(1, 2) = -q1z;
			Q_q2(1, 3) = q1y;
			Q_q2(2, 0) = q1y;
			Q_q2(2, 1) = q1z;
			Q_q2(2, 2) = q1w;
			Q_q2(2, 3) = -q1x;
			Q_q2(3, 0) = q1z;
			Q_q2(3, 1) = -q1y;
			Q_q2(3, 2) = q1x;
			Q_q2(3, 3) = q1w;
		}

		/**
		 * Quaternion product, with Jacobians
		 * \param q1 the first quaternion
		 * \param q2 the second quaternion
		 * \param q the output q = q1*q2
		 * \param Q_q1 the Jacobian of qo wrt q1
		 * \param Q_q2 the Jacobian of qo wrt q2
		 */
		template<class VecQ1, class VecQ2, class VecQ, class MatQ_q1, class MatQ_q2>
		void qProd(const VecQ1 q1, const VecQ2 q2, VecQ q, MatQ_q1 Q_q1, MatQ_q2 Q_q2) {
			q = qProd(q1, q2);
			dQProd_by_dq1(q2, Q_q1);
			dQProd_by_dq2(q1, Q_q2);
		}
	}
}

#endif /* QUATTOOLS_HPP_ */
