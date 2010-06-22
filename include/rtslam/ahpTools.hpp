/**
 * \file ahpTools.hpp
 *
 * \date 13/04/2010
 * \author jsola@laas.fr
 *
 *
 *  This file defines the namespace landmarkAHP in jafar::rtslam.
 *
 * \ingroup rtslam
 */

#include "rtslam/quatTools.hpp"

#ifndef LANDMARKAHP_HPP_
#define LANDMARKAHP_HPP_

namespace jafar {
	namespace rtslam {
		/**
		 * Namespace for operations on Anchored Homogeneous Points.
		 * \ingroup rtslam
		 */
		namespace lmkAHP {
			using namespace ublas;
			using namespace jblas;
			using namespace quaternion;


			/**
			 * Split AHP.
			 * \param p0 the output anchor.
			 * \param m the output director vector
			 * \param rho the homogeneous parameter (inverse-distance)
			 */
			template<class AHP, class P0, class M>
			void split(const AHP & ahp, P0 & p0, M & m, double & rho) {
				p0 = project(ahp, range(0, 3));
				m = project(ahp, range(3, 6));
				rho = ahp(6);
			}


			/**
			 * Compose AHP.
			 *
			 * This is the opposite as split()
			 * \param p0 the output anchor.
			 * \param m the output director vector
			 * \param rho the homogeneous parameter (inverse-distance)
			 */
			template<class P0, class M>
			vec7 compose(const P0 & p0, const M & m, const double rho) {
				vec7 ahp;
				project(ahp, range(0, 3)) = p0;
				project(ahp, range(3, 6)) = m;
				ahp(6) = rho;
				return ahp;
			}

			template<class VF, class Vlf>
			vec fromFrame(const VF & F, const Vlf & ahpf) {


				// split non-trivial chunks of landmark state
				vec3 p0f(subrange(ahpf, 0, 3));
				vec3 mf(subrange(ahpf, 3, 6));


				// transformed landmark in global frame
				vec ahp(7);

				subrange(ahp, 0, 3) = eucFromFrame(F, p0f);
				subrange(ahp, 3, 6) = vecFromFrame(F, mf);
				ahp(6) = ahpf(6);

				return ahp;
			}

			template<class VF, class Vahpf, class Vahp, class MAHP_f, class MAHP_ahpf>
			void fromFrame(const VF & F, const Vahpf & ahpf, Vahp & ahp, MAHP_f & AHP_f, MAHP_ahpf & AHP_ahpf) {
				// split non-trivial chunks of landmark state
				vec3 p0f(subrange(ahpf, 0, 3));
				vec3 mf(subrange(ahpf, 3, 6));
				// destination chunks
				vec3 p0;
				vec3 m;
				mat JAC_33(3, 3), JAC_37(3, 7);
				// Jacobians
				AHP_f.clear();
				AHP_ahpf.clear();
				// transform p0
				eucFromFrame(F, p0f, p0, JAC_37, JAC_33);
				subrange(ahp, 0, 3) = p0;
				subrange(AHP_f, 0, 3, 0, 7) = JAC_37;
				subrange(AHP_ahpf, 0, 3, 0, 3) = JAC_33;
				// transform m
				vecFromFrame(F, mf, m, JAC_37, JAC_33);
				subrange(ahp, 3, 6) = m;
				subrange(AHP_f, 3, 6, 0, 7) = JAC_37;
				subrange(AHP_ahpf, 3, 6, 3, 6) = JAC_33;
				// transform rho
				ahp(6) = ahpf(6);
				AHP_ahpf(6, 6) = 1;
			}

			template<class VF, class Vahp>
			vec toFrame(const VF & F, const Vahp & ahp) {
				// split non-trivial chunks of landmark state
				vec3 p0(subrange(ahp, 0, 3));
				vec3 m(subrange(ahp, 3, 6));
				// transformed landmark in frame F
				vec ahpf(7);
				subrange(ahpf, 0, 3) = eucToFrame(F, p0);
				subrange(ahpf, 3, 6) = vecToFrame(F, m);
				ahpf(6) = ahp(6);

				return ahpf;
			}

			template<class VF, class Vahp, class Vahpf, class MAHPF_f, class MAHPF_ahp>
			void toFrame(const VF & F, const Vahp & ahp, Vahpf & ahpf, MAHPF_f & AHPF_f, MAHPF_ahp & AHPF_ahp) {
				// split non-trivial chunks of landmark state
				vec3 p0(subrange(ahp, 0, 3));
				vec3 m(subrange(ahp, 3, 6));
				// destination chunks
				vec3 p0f;
				vec3 mf;
				mat JAC_33(3, 3), JAC_37(3, 7);
				// Jacobians
				AHPF_f.clear();
				AHPF_ahp.clear();
				// transform p0
				eucToFrame(F, p0, p0f, JAC_37, JAC_33);
				subrange(ahpf, 0, 3) = p0f;
				subrange(AHPF_f, 0, 3, 0, 7) = JAC_37;
				subrange(AHPF_ahp, 0, 3, 0, 3) = JAC_33;
				// transform m
				vecToFrame(F, m, mf, JAC_37, JAC_33);
				subrange(ahpf, 3, 6) = mf;
				subrange(AHPF_f, 3, 6, 0, 7) = JAC_37;
				subrange(AHPF_ahp, 3, 6, 3, 6) = JAC_33;
				// transform rho
				ahpf(6) = ahp(6);
				AHPF_ahp(6, 6) = 1;
			}


			/**
			 * Reparametrize to Euclidean.
			 * \param ahp the anchored homogeneous point to be reparametrized.
			 * \return the Euclidean point.
			 */
			template<class VA>
			vec3 ahp2euc(const VA & ahp) {
				return subrange(ahp, 0, 3) + subrange(ahp, 3, 6) / ahp(6);
			}


			/**
			 * Reparametrize to Euclidean, with Jacobians.
			 * \param ahp the anchored homogeneous point to be reparametrized.
			 * \param euc the returned Euclidean point.
			 * \param EUC_ahp the Jacobian of the conversion.
			 */
			template<class VA, class VE, class ME_a>
			void ahp2euc(const VA & ahp, VE & euc, ME_a & EUC_ahp) {
				euc.assign(ahp2euc(ahp));
				// split non-trivial chunks of landmark state
				vec3 m(subrange(ahp, 3, 6));
				double rho = ahp(6);
				identity_mat I(3);
				subrange(EUC_ahp, 0, 3, 0, 3) = I;
				subrange(EUC_ahp, 0, 3, 3, 6) = I / rho;
				column(EUC_ahp, 6) = -m / rho / rho;
			}


			/**
			 * Bring landmark to bearing-only sensor frame (without range information).
			 *
			 * For a landmark and sensor frame
			 * - ahp = [p0 m rho]
			 * - s = [t q],
			 *
			 * this function computes the chain (See Sola \e etal. PAMI 2010):
			 * - R'(q) * ( m - (t - p0) * rho )
			 *
			 * which is a vector in sensor frame in the direction of the landmark. The range information is lost.
			 *
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \return the bearing-only landmark in sensor frame
			 */
			template<class VS, class VA>
			vec3 toBearingOnlyFrame(const VS & s, const VA & ahp) {
				vec3 p0, m;
				double rho;
				split(ahp, p0, m, rho);
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec3 d = m - (t - p0) * rho;
				return rotateInv(q, d); // OK JS April 1 2010
			}


			/**
			 * Bring landmark to bearing-only sensor frame (give distance information).
			 *
			 * For a landmark and sensor frame
			 * - ahp = [p0 m rho]
			 * - s = [t q],
			 *
			 * this function computes the chain (See Sola \e etal. PAMI 2010):
			 * - v = R'(q) * ( m - (t - p0) * rho )
			 *
			 * which is a vector in sensor frame in the direction of the landmark.
			 *
			 * The range information is recuperated in \a dist as the distance from sensor to landmark.
			 *
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \param v the bearing-only landmark in sensor frame
			 * \param dist the non-observable distance
			 */
			template<class VS, class VA, class VV>
			void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & dist) {
				vec3 p0, m;
				double rho;
				split(ahp, p0, m, rho);
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec3 d = m - (t - p0) * rho;
				v = rotateInv(q, d); // OK JS April 1 2010
				dist = norm_2(v) / rho;
			}


			/**
			 * Bring landmark to bearing-only sensor frame (give distance information).
			 *
			 * For a landmark and sensor frame
			 * - ahp = [p0 m rho]
			 * - s = [t q],
			 *
			 * this function computes the chain (See Sola \e etal. PAMI 2010):
			 * - R'(q) * ( m - (t - p0) * rho )
			 *
			 * which is a vector in sensor frame in the direction of the landmark.
			 *
			 * The range information is recuperated in \a dist as the distance from sensor to landmark.
			 *
			 * and returns the Jacobians wrt s and ahp.
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \param v the bearing-only landmark in sensor frame
			 * \param V_s the Jacobian of \a v wrt \a s
			 * \param V_ahp the Jacobian of \a v wrt \a ahp
			 */
			template<class VS, class VA, class VV, class MV_s, class MV_a>
			void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & dist, MV_s & V_s, MV_a & V_ahp) {
				vec3 p0, m;
				double rho;
				mat34 V_q;
				mat33 V_d;
				// value
				split(ahp, p0, m, rho);
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec3 d = m - (t - p0) * rho; //    before rotation
				rotateInv(q, d, v, V_q, V_d); //   obtain v
				dist = norm_2(v) / rho; //     		 obtain non-observable distance
				// Jacobians
				mat33 V_p0;
				V_p0 = V_d * rho;
				//				mat V_rho(3,1); // This comments only for Jacobian reference...
				//				V_m = V_d;
				//				V_rho = prod(V_d, (p0 - t));
				//				V_t = - V_d * rho;
				V_s.clear();
				V_ahp.clear();
				subrange(V_s, 0, 3, 0, 3) = -V_p0; //       dv / dt
				subrange(V_s, 0, 3, 3, 7) = V_q; //         dv / dq
				subrange(V_ahp, 0, 3, 0, 3) = V_p0; //      dv / dp0
				subrange(V_ahp, 0, 3, 3, 6) = V_d; //       dv / dm
				column(V_ahp, 6) = prod(V_d, (p0 - t)); //  dv / drho   // OK JS April 1 2010
			}


			/**
			 * AHP landmark from bearing-only retro-projection.
			 * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
			 * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
			 *
			 * It uses the formula (See Sola \e etal. PAMI 2010):
			 * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
			 *
			 * \param s the sensor frame s = [t ; q]
			 * \param v the retro-projected director vector in sensor frame
			 * \param rho the prior, proportional to inverse-distance
			 * \return the AHP landmark.
			 */
			template<class VS, class VLS>
			vec7 fromBearingOnlyFrame(const VS & s, const VLS & v, const double _rho) {
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec7 ahp;

				subrange(ahp, 0, 3) = t;
				subrange(ahp, 3, 6) = rotate(q, v);
				ahp(6) = _rho * norm_2(v);
				return ahp; // OK JS April 1 2010
			}


			/**
			 * AHP landmark from bearing-only retro-projection, with Jacobians.
			 * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
			 * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
			 *
			 * It uses the formula (See Sola \e etal. PAMI 2010):
			 * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
			 *
			 * \param s the sensor frame
			 * \param v the retro-projected director vector in sensor frame
			 * \param rho the prior, proportional to inverse-distance
			 * \param ahp the AHP landmark.
			 * \param AHP_s the Jacobian wrt \a s
			 * \param AHP_v the Jacobian wrt \a v
			 * \param AHP_rho the Jacobian wrt \a rho
			 */
			template<class VS, class VLS, class VA, class MA_s, class MA_v, class MA_rho>
			void fromBearingOnlyFrame(const VS & s, const VLS & v, const double _rho, VA & ahp, MA_s & AHP_s, MA_v & AHP_v,
			    MA_rho & AHP_rho) {
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec3 m;
				mat34 M_q;
				mat33 M_v;
				mat NV_v(1, 3);

				rotate(q, v, m, M_q, M_v);
				double nv = norm_2(v);

				subrange(ahp, 0, 3) = t; // p0  = t
				subrange(ahp, 3, 6) = m; // m   = R(q) * v
				ahp(6) = _rho * nv; //              rho = ||v|| * _rho
				ublasExtra::norm_2Jac<3>(v, NV_v); // drho / dv = d||v||/dv * _rho

				// Jacobians
				identity_mat I(3);
				AHP_s.clear();
				AHP_v.clear();
				AHP_rho.clear();
				subrange(AHP_s, 0, 3, 0, 3) = I; //             dp0 / dt
				subrange(AHP_s, 3, 6, 3, 7) = M_q; //            dm / dq
				subrange(AHP_v, 3, 6, 0, 3) = M_v; //            dm / dv
				subrange(AHP_v, 6, 7, 0, 3) = _rho * NV_v; //  drho / dv
				AHP_rho(6, 0) = nv; //                         drho / drho
			} // OK JS April 1 2010

			template<class VA, class VS, class MA_s>
			double linearityScore(const VA & ahp, const VS & sen, const MA_s & RHO){

					vec3 euc;

					euc = ahp2euc(ahp);

					vec3 hw = euc - subrange(sen, 0, 3);

					double sigma_rho = sqrt(RHO(7,7));

					double sigma_d   = sigma_rho/ahp(7)^2;
					double d1        = norm_2(hw);
					double cos_a     =  trans(subrange(sen, 3, 6)) * hw/d1;

					return 4*sigma_d/d1*abs(cos_a);



		}

		} // namespace landmarkAHP


	}
}

#endif /* LANDMARKAHP_HPP_ */
