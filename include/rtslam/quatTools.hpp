/**
 * quatTools.hpp
 *
 *  Created on: 20/02/2010
 *      Author: jsola
 *
 * \file quatTools.hpp
 * \author jsola
 * File defining geometric operations with quaternions.
 * Quaternions operate on vectors and other quaternions.
 * Functions for Jacobian computations are also provided.
 *
 * \ingroup rtslam
 */

#ifndef QUATTOOLS_HPP_
#define QUATTOOLS_HPP_

#include "jmath/ublasExtra.hpp"
#include "jmath/jblas.hpp"

namespace jafar {
	namespace rtslam {
		/**
		 * Namespace for quaternion algebra functions.
		 * \ingroup rtslam
		 */
		namespace quaternion {

			using namespace jblas;
			using namespace jafar::jmath;


			/**
			 * Conjugate of quaternion.
			 * \param q The input quaternion [qw, qx, qy, qz].
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
			 * Jacobian of conjugated quaternion wrt input quaternion.
			 *
			 * Jac = QC_q = dqc/dq.
			 * \param q the input quaternion [qw, qx, qy, qz]
			 * \param QC_q the Jacobian.
			 */
			template<class VecQ, class MatQC_q>
			void q2qc_by_dq(const VecQ & q, MatQC_q & QC_q) {
				QC_q.clear();
				QC_q(0, 0) = 1.0;
				QC_q(1, 1) = -1.0;
				QC_q(2, 2) = -1.0;
				QC_q(3, 3) = -1.0;
			}


			/**
			 * Conjugate of a quaternion, with Jacobians.
			 */
			template<class VecQ, class VecQc, class MatQC_q>
			void q2qc(const VecQ & q, VecQc & qc, MatQC_q & QC_q) {
				qc = q2qc(q);
				q2qc_by_dq(q, QC_q);
			}


			/**
			 * Rotation matrix from quaternion.
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
			 * The transposed rotation matrix from a quaternion.
			 * \param q the quaternion [qw, qx, qy, qz].
			 * \return the transposed rotation matrix.
			 */
			template<class VecQ>
			mat33 q2Rt(const VecQ & q) {
				return q2R(q2qc(q));
			}


			/**
			 * Rotate a vector from given quaternion.
			 * \param q the quaternion [qw, qx, qy, qz].
			 * \param v the vector.
			 * \return the rotated vector.
			 */
			template<class VecQ, class Vec>
			vec3 rotate(const VecQ & q, const Vec & v) {
				using namespace ublas;
				return prod(q2R(q), v);
			}


			/**
			 * Jacobian of vector rotation from quaternion, wrt quaternion.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \param v the vector
			 * \param VO_q the Jacobian of R(q)*v wrt q
			 */
			template<class VecQ, class Vec, class MatVO_q>
			void rotate_by_dq(const VecQ & q, const Vec & v, MatVO_q & VO_q) {
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
			 * Jacobian of vector rotation from quaternion, wrt vector.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \param VO_v = R(q) the Jacobian of R(q)*v wrt v
			 */
			template<class VecQ, class MatVO_v>
			void rotate_by_dv(const VecQ & q, MatVO_v & VO_v) {
				VO_v = q2R(q);
			}


			/**
			 * Rotate a vector from given quaternion, with all Jacobians.
			 * \param q the quaternion [qw, qx, qy, qz];
			 * \param v the vector
			 * \param vo the rotated vector
			 * \param VO_q the Jacobian wrt q
			 * \param VO_v the Jacobian wrt v
			 */
			template<class VecQ, class Vec, class VecO, class MatVO_q, class MatVO_v>
			void rotate(const VecQ & q, const Vec & v, VecO & vo, MatVO_q & VO_q, MatVO_v & VO_v) {
				vo = rotate(q, v);
				rotate_by_dq(q, v, VO_q);
				rotate_by_dv(q, VO_v);
			}


			/**
			 * Rotate inversely a vector from given quaternion.
			 * \param q the quaternion [qw, qx, qy, qz];
			 * \param v the vector
			 * \return the rotated vector
			 */
			template<class VecQ, class Vec>
			vec3 rotateInv(const VecQ & q, const Vec & v) {
				using namespace ublas;
				return prod(q2Rt(q), v);
			}


			/**
			 * Jacobian of inverse vector rotation from quaternion, wrt quaternion.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \param v the vector
			 * \param VO_q the Jacobian of R(q)'*v wrt q
			 */
			template<class VecQ, class Vec, class MatVO_q>
			void rotateInv_by_dq(const VecQ & q, const Vec & v, MatVO_q & VO_q) {
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
			 * Jacobian of vector inverse rotation from quaternion, wrt vector.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \param VO_v = R(q) the Jacobian of R(q)'*v wrt v
			 */
			template<class VecQ, class MatVO_v>
			void rotateInv_by_dv(const VecQ & q, MatVO_v & VO_v) {
				VO_v = q2Rt(q);
			}


			/**
			 * Rotate inversely a vector from given quaternion, with all Jacobians.
			 * \param q the quaternion [qw, qx, qy, qz];
			 * \param v the vector
			 * \param vo the rotated vector
			 * \param VO_q the Jacobian wrt q
			 * \param VO_v the Jacobian wrt v
			 */
			template<class VecQ, class Vec, class VecO, class MatVO_q, class MatVO_v>
			void rotateInv(const VecQ & q, const Vec & v, VecO & vo, MatVO_q & VO_q, MatVO_v & VO_v) {
				VO_v = q2Rt(q); // this is Rt !
				vo = prod(VO_v, v);
				rotateInv_by_dq(q, v, VO_q);
			}


			/**
			 * Quaternion product.
			 * \param q1 the first quaternion [qw, qx, qy, qz]
			 * \param q2 the second quaternion
			 * \return the product q1**q2, where ** means quaternion product
			 */
			template<class VecQ1, class VecQ2>
			vec4 qProd(const VecQ1 & q1, const VecQ2 & q2) {
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
			 * Jacobian of quaternion product wrt first quaternion, d(q1**q2)/dq1.
			 *
			 * This Jacobian only depends on q2.
			 * \param q2 the second quaternion [qw, qx, qy, qz] of the product
			 * \param Q_q1 the output Jacobian
			 */
			template<class VecQ2, class MatQ_q1>
			void qProd_by_dq1(const VecQ2 & q2, MatQ_q1 & Q_q1) {


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
			 * Jacobian of quaternion product wrt second quaternion, d(q1**q2)/dq2.
			 *
			 * This Jacobian only depends on q1.
			 * \param q1 the first quaternion [qw, qx, qy, qz] of the product
			 * \param Q_q2 the output Jacobian
			 */
			template<class VecQ1, class MatQ_q2>
			void qProd_by_dq2(const VecQ1 & q1, MatQ_q2 & Q_q2) {


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
			 * Quaternion product, with Jacobians.
			 * \param q1 the first quaternion [qw, qx, qy, qz]
			 * \param q2 the second quaternion
			 * \param q the output q = q1**q2
			 * \param Q_q1 the Jacobian of q wrt q1
			 * \param Q_q2 the Jacobian of q wrt q2
			 */
			template<class VecQ1, class VecQ2, class VecQ, class MatQ_q1, class MatQ_q2>
			void qProd(const VecQ1 & q1, const VecQ2 & q2, VecQ & q, MatQ_q1 & Q_q1, MatQ_q2 & Q_q2) {
				q = qProd(q1, q2);
				qProd_by_dq1(q2, Q_q1);
				qProd_by_dq2(q1, Q_q2);
			}


			/**
			 * Quaternion from rotation vector.
			 * \param v the rotation vector
			 * \return the quaternion [qw, qx, qy, qz]
			 */
			template<class Vec>
			vec4 v2q(const Vec & v) {
				double a = boost::numeric::ublas::norm_2(v);
				double san = sin(a / 2) / a;
				vec4 q;
				q(0) = cos(a / 2);
				q(1) = v(0) * san;
				q(2) = v(1) * san;
				q(3) = v(2) * san;
				return q;
			}


			/**
			 * Jacobian of quaternion wrt rotation vector.
			 * \param v the rotation vector
			 * \param Q_v the output Jacobian
			 */
			template<class Vec, class MatQ_v>
			void v2q_by_dv(const Vec & v, MatQ_v & Q_v) {

				double a = norm_2(v);

				if (a > jmath::ublasExtra::details::EPSILON) {


					// u = v/norm(v) and U_v
					vec3 u(v);
					ublasExtra::normalize(u);
					mat U_v(3, 3);
					jmath::ublasExtra::normalizeJac(v, U_v);


					// Av = u';
					mat A_v(1, 3);
					row(A_v, 0) = u;


					//	Qa = [  -sin(a/2)/2
					//				 u*cos(a/2)/2];
					mat Q_a(4, 1);
					double sa2 = sin(a / 2);
					double ca22 = cos(a / 2) / 2;
					Q_a(0, 0) = -sa2 / 2;
					Q_a(1, 0) = u(0) * ca22;
					Q_a(2, 0) = u(1) * ca22;
					Q_a(3, 0) = u(2) * ca22;


					//	Qu = [0 0 0;eye(3)*sin(a/2)];
					mat Q_u(4, 3);
					Q_u.clear();
					Q_u(1, 0) = sa2;
					Q_u(2, 1) = sa2;
					Q_u(3, 2) = sa2;


					// chain rule
					Q_v = prod(Q_a, A_v) + prod(Q_u, U_v);

				}
				else {
					//				v = -v / 4;
					//				v /= -4.0;
					Q_v.clear();
					Q_v(0, 0) = v(0) / 4;
					Q_v(0, 1) = v(1) / 4;
					Q_v(0, 2) = v(2) / 4;
					Q_v(1, 0) = 0.5;
					Q_v(2, 1) = 0.5;
					Q_v(3, 2) = 0.5;
				}
			}


			/** Rotation vector to quaternion conversion, with Jacobian.
			 * \param v the rotation vector
			 * \param q the output quaternion [qw, qx, qy, qz]
			 * \param Q_v the Jacobian
			 */
			template<class Vec, class VecQ, class MatQ_v>
			void v2q(const Vec & v, VecQ & q, MatQ_v & Q_v) {
				q = v2q(v);
				v2q_by_dv(v, Q_v);
			}


			/**
			 * Quaternion from Euler angles.
			 * \param e the Euler angles [roll, pitch, yaw]
			 * \return the quaternion [qw, qx, qy, qz]
			 */
			template<class Vec>
			vec4 e2q(const Vec & e) {
				vec3 ex;
				vec3 ey;
				vec3 ez;
				ex.clear();
				ex(0) = e(0);
				vec4 qx = v2q(ex);
				ey.clear();
				ey(1) = e(1);
				vec4 qy = v2q(ey);
				ez.clear();
				ez(2) = e(2);
				vec4 qz = v2q(ez);

				vec4 q;
				q = qProd(qProd(qz, qy), qx);
				return q;
			}


			/**
			 * Jacobian of quaternion wrt Euler angles.
			 * \param e the Euler angles [roll, pitch, yaw]
			 * \param Q_e the output Jacobian
			 */
			template<class Vec, class MatQ_e>
			void e2q_by_de(const Vec & e, MatQ_e & Q_e) {
				double sr = sin(e(0) / 2);
				double sp = sin(e(1) / 2);
				double sy = sin(e(2) / 2);

				double cr = cos(e(0) / 2);
				double cp = cos(e(1) / 2);
				double cy = cos(e(2) / 2);

				Q_e(0, 0) = (-cy * cp * sr + sy * sp * cr) / 2;
				Q_e(0, 1) = (-cy * sp * cr + sy * cp * sr) / 2;
				Q_e(0, 2) = (-sy * cp * cr + cy * sp * sr) / 2;
				Q_e(1, 0) = (cy * cp * cr + sy * sp * sr) / 2;
				Q_e(1, 1) = (-cy * sp * sr - sy * cp * cr) / 2;
				Q_e(1, 2) = (-sy * cp * sr - cy * sp * cr) / 2;
				Q_e(2, 0) = (-cy * sp * sr + sy * cp * cr) / 2;
				Q_e(2, 1) = (cy * cp * cr - sy * sp * sr) / 2;
				Q_e(2, 2) = (-sy * sp * cr + cy * cp * sr) / 2;
				Q_e(3, 0) = (-sy * cp * sr - cy * sp * cr) / 2;
				Q_e(3, 1) = (-cy * cp * sr - sy * sp * cr) / 2;
				Q_e(3, 2) = (cy * cp * cr + sy * sp * sr) / 2;
			}


			/** Euler angles to quaternion conversion, with Jacobian.
			 * \param e the Euler angles [roll, pitch, yaw]
			 * \param q the output quaternion [qw, qx, qy, qz]
			 * \param Q_e the Jacobian
			 */
			template<class Vec, class VecQ, class MatQ_e>
			void e2q(const Vec & e, VecQ & q, MatQ_e & Q_e) {
				q = e2q(e);
				e2q_by_de(e, Q_e);
			}


			/**
			 * Quaternion to Euler angles conversion.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \return the Euler angles [roll, pitch, yaw]
			 */
			template<class VecQ>
			vec3 q2e(const VecQ & q) {
				vec3 e;

				double y1 = 2 * q(2) * q(3) + 2 * q(0) * q(1);
				double x1 = q(0) * q(0) - q(1) * q(1) - q(2) * q(2) + q(3) * q(3);
				double z2 = -2 * q(1) * q(3) + 2 * q(0) * q(2);
				double y3 = 2 * q(1) * q(2) + 2 * q(0) * q(3);
				double x3 = q(0) * q(0) + q(1) * q(1) - q(2) * q(2) - q(3) * q(3);
				e(0) = atan2(y1, x1);
				e(1) = asin(z2);
				e(2) = atan2(y3, x3);
				return e;
			}


			/**
			 * Quaternion-to-Euler conversion, with Jacobian.
			 * \param q the quaternion [qw, qx, qy, qz]
			 * \param e the Euler angles [roll, pitch, yaw]
			 * \param E_q the Jacobian
			 */
			template<class VecQ, class VecE, class MatE_q>
			void q2e(const VecQ & q, VecE & e, MatE_q & E_q) {

				double y1 = 2 * q(2) * q(3) + 2 * q(0) * q(1);
				double x1 = q(0) * q(0) - q(1) * q(1) - q(2) * q(2) + q(3) * q(3);
				double z2 = -2 * q(1) * q(3) + 2 * q(0) * q(2);
				double y3 = 2 * q(1) * q(2) + 2 * q(0) * q(3);
				double x3 = q(0) * q(0) + q(1) * q(1) - q(2) * q(2) - q(3) * q(3);


				// Euler angles
				e(0) = atan2(y1, x1);
				e(1) = asin(z2);
				e(2) = atan2(y3, x3);


				// Jacobians start here
				vec4 dx1dq;
				vec4 dy1dq;
				vec4 dz2dq;
				vec4 dx3dq;
				vec4 dy3dq;


				// derivatives of XYZ wrt q
				dx1dq(0) = 2 * q(0);
				dx1dq(1) = -2 * q(1);
				dx1dq(2) = -2 * q(2);
				dx1dq(3) = 2 * q(3);
				dy1dq(0) = 2 * q(1);
				dy1dq(1) = 2 * q(0);
				dy1dq(2) = 2 * q(3);
				dy1dq(3) = 2 * q(2);
				dz2dq(0) = 2 * q(2);
				dz2dq(1) = -2 * q(3);
				dz2dq(2) = 2 * q(0);
				dz2dq(3) = -2 * q(1);
				dx3dq(0) = 2 * q(0);
				dx3dq(1) = 2 * q(1);
				dx3dq(2) = -2 * q(2);
				dx3dq(3) = -2 * q(3);
				dy3dq(0) = 2 * q(3);
				dy3dq(1) = 2 * q(2);
				dy3dq(2) = 2 * q(1);
				dy3dq(3) = 2 * q(0);

				double sx2y2 = (x1 * x1 + y1 * y1);
				double sx3y3 = (x3 * x3 + y3 * y3);


				// derivatives of E wrt XYZ
				double de1dx1 = -y1 / sx2y2;
				double de1dy1 = x1 / sx2y2;
				double de2dz2 = 1 / sqrt(1 - z2 * z2);
				double de3dx3 = -y3 / sx3y3;
				double de3dy3 = x3 / sx3y3;


				// the chain rule, de/dq = de/dXYZ * dXYZ/dq
				row(E_q, 0) = de1dx1 * dx1dq + de1dy1 * dy1dq;
				row(E_q, 1) = de2dz2 * dz2dq;
				row(E_q, 2) = de3dx3 * dx3dq + de3dy3 * dy3dq;
			}


			/**
			 * To-frame transformation for Euclidean points.
			 * \param F frame
			 * \param p point in global frame
			 * \return the point in frame F
			 */
			template<class VecF, class Pnt>
			vec3 eucToFrame(const VecF & F, const Pnt & p) {

				using namespace ublas;

				vec4 q = project(F, range(3, 7));
				vec3 t = project(F, range(0, 3));
				vec3 v = p - t;
				vec3 pf;
				pf = rotateInv(q, v);
				return pf;
			}


			/**
			 * To-frame transformation for Euclidean points, with Jacobians.
			 * \param F frame
			 * \param p point in global frame
			 * \param pf the point in frame F
			 * \param PF_f the Jacobian wrt F
			 * \param PF_p the Jacobian wrt p
			 */
			template<class VecF, class Pnt, class PntF, class MatPF_f, class MatPF_p>
			void eucToFrame(const VecF & F, const Pnt & p, PntF & pf, MatPF_f & PF_f, MatPF_p & PF_p) {

				using namespace ublas;

				vec4 q = project(F, range(3, 7));
				vec3 t = project(F, range(0, 3));
				vec3 v = p - t;
				mat_range PF_q(PF_f, range(0, 3), range(3, 7));
				rotateInv(q, v, pf, PF_q, PF_p);
				project(PF_f, range(0, 3), range(0, 3)) = -PF_p;
			}


			/**
			 * To-frame transformation for vectors
			 * \param F frame
			 * \param v vector in global frame
			 * \return the vector in frame F
			 */
			template<class VecF, class Vec>
			vec3 vecToFrame(const VecF & F, const Vec & v) {
				return rotateInv(subrange(F, 3, 7), v);
			}


			/**
			 * To-frame transformation for vectors, with Jacobians.
			 * \param F frame
			 * \param v vector in global frame
			 * \param vf the point in frame F
			 * \param VF_f the Jacobian wrt F
			 * \param VF_v the Jacobian wrt v
			 */
			template<class VecF, class Vec, class Vecf, class MatVF_f, class MatVF_v>
			void vecToFrame(const VecF & F, const Vec & v, Vecf & vf, MatVF_f & VF_f, MatVF_v & VF_v) {

				using namespace ublas;

				vec4 q = project(F, range(3, 7));
				mat_range VF_q(VF_f, range(0, 3), range(3, 7));
				rotateInv(q, v, vf, VF_q, VF_v);
				project(VF_f, range(0, 3), range(0, 3)) = zero_mat(3, 3);
			}


			/**
			 * From-frame transformation for Euclidean points.
			 * \param F frame
			 * \param pf point in frame F
			 * \return the point in global frame
			 */
			template<class VecF, class Pnt>
			vec3 eucFromFrame(const VecF & F, const Pnt & pf) {

				using namespace ublas;

				return rotate(project(F, range(3, 7)), pf) + project(F, range(0, 3));
			}


			/**
			 * From-frame transformation for Euclidean points, with Jacobians.
			 * \param F frame
			 * \param pf point in frame F
			 * \param p the point in global frame
			 * \param P_f the Jacobian wrt F
			 * \param P_pf the Jacobian wrt pf
			 */
			template<class VecF, class PntF, class Pnt, class MatP_f, class MatP_pf>
			void eucFromFrame(const VecF & F, const PntF & pf, Pnt & p, MatP_f & P_f, MatP_pf & P_pf) {

				using namespace ublas;

				vec4 q = project(F, range(3, 7));
				vec3 t = project(F, range(0, 3));
				mat_range P_q(P_f, range(0, 3), range(3, 7));
				rotate(q, pf, p, P_q, P_pf);
				p += t;
				project(P_f, range(0, 3), range(0, 3)) = identity_mat(3); // dp/dt = I
			}


			/**
			 * From-frame transformation for vectors.
			 * \param F frame
			 * \param vf vector in frame F
			 * \return the vector in global frame
			 */
			template<class VecF, class Vec>
			vec3 vecFromFrame(const VecF & F, const Vec & vf) {

				using namespace ublas;

				return rotate(subrange(F, 3, 7), vf);
			}


			/**
			 * From-frame transformation for vectors, with Jacobians.
			 * \param F frame
			 * \param vf vector in frame F
			 * \param v the point in global frame
			 * \param V_f the Jacobian wrt F
			 * \param V_vf the Jacobian wrt vf
			 */
			template<class VecF, class Vecf, class Vec, class MatV_f, class MatV_vf>
			void vecFromFrame(const VecF & F, const Vecf & vf, Vec & v, MatV_f & V_f, MatV_vf & V_vf) {
				using namespace ublas;
				vec4 q = project(F, range(3, 7));
				project(V_f, range(0, 3), range(0, 3)) = zero_mat(3, 3); // dv/dt = 0
				mat_range V_q(V_f, range(0, 3), range(3, 7));
				rotate(q, vf, v, V_q, V_vf);
			}


			/**
			 * Compose frames.
			 * \param G the global frame
			 * \param F the local frame (expressed in G)
			 * \return the composed frame GoF
			 */
			template<class VecG, class VecF>
			jblas::vec7 composeFrames(VecG & G, VecF & F) {
				vec4 q1 = subrange(G, 3, 7);
				vec3 t2 = subrange(F, 0, 3);
				vec4 q2 = subrange(F, 3, 7);
				vec3 t = eucFromFrame(G, t2);
				vec4 q = qProd(q1, q2);
				vec7 H;
				subrange(H, 0, 3) = t;
				subrange(H, 3, 7) = q;
				return H;
			}


			/**
			 * Jacobian of frame composition wrt global frame.
			 * \param G the global frame
			 * \param L the local frame
			 * \param C_g the Jacobian of C wrt G
			 */
			template<class VecG, class VecL, class MatC_g>
			void composeFrames_by_dglobal(const VecG & G, const VecL & L, MatC_g & C_g) {
					/*
					 * C_g = [ I_3   rotate_by_dq ]
					 *       [  0    qProd_by_dq1 ]
					 */
				vec4 qg = subrange(G, 3, 7);
				vec3 tl = subrange(L, 0, 3);
				vec4 ql = subrange(L, 3, 7);
				mat T_qg(3,4);
				mat Q_qg(4,4);
				C_g.clear();
				ublas::subrange(C_g, 0, 3, 0, 3) = jblas::identity_mat(3);
				rotate_by_dq(qg, tl, T_qg);
				ublas::subrange(C_g, 0, 3, 3, 7) = T_qg;
				qProd_by_dq1(ql, Q_qg);
				ublas::subrange(C_g, 3, 7, 3, 7) = Q_qg;
			}


			/**
			 * Jacobian of frame composition wrt global frame.
			 * \param G the global frame
			 * \param L the local frame
			 * \param C_g the Jacobian of C wrt G
			 */
			template<class VecG, class VecL, class MatC_l>
			void composeFrames_by_dlocal(const VecG & G, const VecL & L, MatC_l & C_l) {
					/*
					 * C_l = [ rotate_by_dv       0      ]
					 *       [      0       qProd_by_dq2 ]
					 */
				vec4 qg = subrange(G, 3, 7);
				mat_range T_tl(C_l, 0, 3, 0, 3);
				mat_range Q_ql(C_l, 4, 7, 4, 7);
				C_l.clear();
				rotate_by_dv(qg, T_tl);
				qProd_by_dq2(qg, Q_ql);
			}


			/**
			 * Compose frames, give Jacobians.
			 * \param G the global frame
			 * \param L the local frame
			 * \param C the composed frame GoL
			 * \param C_g the Jacobian of C wrt G
			 * \param C_l the Jacobian wrt L
			 */
			template<class VecG, class VecL, class VecC, class MatC_g, class MatC_l>
			void composeFrames(const VecG & G, const VecL & L, VecC & C, MatC_g & C_g, MatC_l & C_l) {
				vec4 qg = subrange(G, 3, 7);
				vec3 tl = subrange(L, 0, 3);
				vec4 ql = subrange(L, 3, 7);
				mat Q_qg(4, 4), Q_ql(4, 4);
				mat T_g(3, 7), T_tl(3, 3);
				vec3 t;
				vec4 q;
				eucFromFrame(G, tl, t, T_g, T_tl);
				qProd(qg, ql, q, Q_qg, Q_ql);
				subrange(C, 0, 3) = t;
				subrange(C, 3, 7) = q;
				C_g.clear();
				subrange(C_g, 0, 3, 0, 7) = T_g;
				subrange(C_g, 3, 7, 3, 7) = Q_qg;
				C_l.clear();
				subrange(C_l, 0, 3, 0, 3) = T_tl;
				subrange(C_l, 3, 7, 3, 7) = Q_ql;
			}


			/**
			 * Invert frame.
			 * \param F the frame to invert.
			 * \return the inverse of \a F.
			 */
			template<class VecF>
			jblas::vec7 invertFrame(VecF & F) {
				vec3 t = subrange(F, 0, 3);
				vec4 q = subrange(F, 3, 7);
				vec7 res;
				subrange(res, 0, 3) = -rotateInv(q, t);
				subrange(res, 3, 7) = q2qc(q);
				return res;
			}


			/**
			 * Invert frame, with Jacobian.
			 * \param F the frame to invert.
			 * \param I the inverse of \a F.
			 * \param I_f the Jacobian
			 */
			template<class VecF, class VecI, class MatI_f>
			void invertFrame(VecF & F, VecI & I, MatI_f & I_f) {

				vec3 Ft = subrange(F, 0, 3);
				vec4 Fq = subrange(F, 3, 7);
				vec3 nt;
				vec4 q;
				jblas::mat NT_q(3, 4);
				jblas::mat NT_t(3, 3);
				jblas::mat Q_q(4, 4);

				rotateInv(Fq, Ft, nt, NT_q, NT_t);
				q2qc(Fq, q, Q_q);
				I_f.clear();
				subrange(I_f, 0, 3, 0, 3) = -NT_t;
				subrange(I_f, 0, 3, 3, 7) = -NT_q;
				subrange(I_f, 3, 7, 3, 7) = Q_q;

			}

		}
	}
}

#endif /* QUATTOOLS_HPP_ */
