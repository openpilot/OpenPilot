/**
 * \file landmarkAnchoredHomogeneousPoint.hpp
 * \author jsola
 *  Created on: 14/02/2010
 *
 * Header file for anchored homogeneous point
 * \ingroup rtslam
 */

#ifndef LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_
#define LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_

#include "boost/shared_ptr.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		class LandmarkAnchoredHomogeneousPoint;
		typedef boost::shared_ptr<LandmarkAnchoredHomogeneousPoint> ahp_ptr_t;


		/**
		 * Namespace for operations on Anchored Homogeneous Points.
		 * \ingroup rtslam
		 */
		namespace landmarkAHP {
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
			 * Bring landmark to bearing-only sensor frame (give inverse-distance information).
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
			 * The range information is recuperated in \a invDist as the inverse of the distance from sensor to landmark.
			 *
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \param v the bearing-only landmark in sensor frame
			 * \param invDist the inverse of the non-observable distance
			 */
			template<class VS, class VA, class VV>
			void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & invDist) {
				vec3 p0, m;
				double rho;
				split(ahp, p0, m, rho);
				vec3 t = subrange(s, 0, 3);
				vec4 q = subrange(s, 3, 7);
				vec3 d = m - (t - p0) * rho;
				v = rotateInv(q, d); // OK JS April 1 2010
				invDist = rho / norm_2(v);
			}


			/**
			 * Bring landmark to bearing-only sensor frame (give inverse-distance information).
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
			 * The range information is recuperated in \a invDist as the inverse of the distance from sensor to landmark.
			 *
			 * and returns the Jacobians wrt s and ahp.
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \param v the bearing-only landmark in sensor frame
			 * \param V_s the Jacobian of \a v wrt \a s
			 * \param V_ahp the Jacobian of \a v wrt \a ahp
			 */
			template<class VS, class VA, class VV, class MV_s, class MV_a>
			void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & invDist, MV_s & V_s, MV_a & V_ahp) {
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
				invDist = rho / norm_2(v); //      obtain invDist
				// Jacobians
				mat33 V_p0;
				V_p0 = V_d * rho;
				//				mat V_rho(3,1); // This comments only for Jacobian reference...
				//				V_m = V_d;
				//				V_rho = prod(V_d, (p0 - t));
				//				V_t = - V_d * rho;
				V_s.clear();
				V_ahp.clear();
				subrange(V_s, 0, 3, 0, 3) = -V_p0; //      dv / dt
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
			 * \param s the sensor frame
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

		} // namespace landmarkAHP


		/**
		 * Class for anchored homogeneous 3D points
		 * \ingroup rtslam
		 */
		class LandmarkAnchoredHomogeneousPoint: public LandmarkAbstract {
			public:


				/**
				 * Constructor from map
				 */
				LandmarkAnchoredHomogeneousPoint(const map_ptr_t mapPtr);

				static size_t size(void) {
					return 7;
				}


				/**
				 * From-frame transform.
				 * \param F a frame to transform from
				 * \param ahpf an AHP point in F-frame
				 * \return the AHP point in global frame
				 */
				template<class VF, class Vlf>
				static vec fromFrame(const VF & F, const Vlf & ahpf) {
					return landmarkAHP::fromFrame(F, ahpf);
				}


				/**
				 * From-frame transform, with Jacobians.
				 * \param F a frame to transform from
				 * \param ahpf an AHP point in F-frame
				 * \param ahp the AHP point in global frame
				 * \param AHP_f the Jacobian of \a ahp wrt \a F
				 * \param AHP_ahpf the Jacobians of \a ahp wrt \a ahpf
				 */
				template<class VF, class Vahpf, class Vahp, class MAHP_f, class MAHP_ahpf>
				static void fromFrame(const VF & F, const Vahpf & ahpf, Vahp & ahp, MAHP_f & AHP_f, MAHP_ahpf & AHP_ahpf) {
					landmarkAHP::fromFrame(F, ahpf, ahp, AHP_f, AHP_ahpf);
				}


				/**
				 * To-frame transform.
				 * \param F a frame to transform to
				 * \param ahp an AHP point in global frame
				 * \return the AHP point in F-frame
				 */
				template<class VF, class Vahp>
				static vec toFrame(const VF & F, const Vahp & ahp) {
					return landmarkAHP::toFrame(F, ahp);
				}


				/**
				 * To-frame transform, with Jacobians.
				 * \param F a frame to transform to
				 * \param ahp an AHP point in global frame
				 * \param ahpf the AHP point in F-frame
				 * \param AHPF_f the Jacobian of \a ahpf wrt \a F
				 * \param AHPF_ahp the Jacobians of \a ahpf wrt \a ahp
				 */
				template<class VF, class Vahp, class Vahpf, class MAHPF_f, class MAHPF_ahp>
				static void toFrame(const VF & F, const Vahp & ahp, Vahpf & ahpf, MAHPF_f & AHPF_f, MAHPF_ahp & AHPF_ahp) {
					landmarkAHP::toFrame(F, ahp, ahpf, AHPF_f, AHPF_ahp);
				}


				/**
				 * Reparametrize to Euclidean.
				 * \param ahp the anchored homogeneous point to be reparametrized.
				 * \return the Euclidean point.
				 */
				template<class VA>
				static vec3 ahp2euc(const VA & ahp) {
					return landmarkAHP::ahp2euc(ahp);
				}


				/**
				 * Reparametrize to Euclidean, with Jacobians.
				 * \param ahp the anchored homogeneous point to be reparametrized.
				 * \param euc the returned Euclidean point.
				 * \param EUC_ahp the Jacobian of the conversion.
				 */
				template<class VA, class VE, class ME_a>
				static void ahp2euc(const VA & ahp, VE & euc, ME_a & EUC_ahp) {
					landmarkAHP::ahp2euc(ahp, euc, EUC_ahp);
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
				static vec3 toBearingOnlyFrame(const VS & s, const VA & ahp) {
						return landmarkAHP::toBearingOnlyFrame(s, ahp);
				}

				/**
				 * Bring landmark to bearing-only sensor frame (give inverse-distance information).
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
				 * The range information is recuperated in \a invDist as the inverse of the distance from sensor to landmark.
				 *
				 * \param s the sensor frame
				 * \param ahp the AHP landmark
				 * \param v the bearing-only landmark in sensor frame
				 * \param invDist the inverse of the non-observable distance
				 */
				template<class VS, class VA, class VV>
				static void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & invDist) {
						landmarkAHP::toBearingOnlyFrame(s, ahp, v, invDist);
				}

				/**
				 * Bring landmark to bearing-only sensor frame (give inverse-distance information).
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
				 * The range information is recuperated in \a invDist as the inverse of the distance from sensor to landmark.
				 *
				 * and returns the Jacobians wrt s and ahp.
				 * \param s the sensor frame
				 * \param ahp the AHP landmark
				 * \param v the bearing-only landmark in sensor frame
				 * \param V_s the Jacobian of \a v wrt \a s
				 * \param V_ahp the Jacobian of \a v wrt \a ahp
				 */
				template<class VS, class VA, class VV, class MV_s, class MV_a>
				static void toBearingOnlyFrame(const VS & s, const VA & ahp, VV & v, double & invDist, MV_s & V_s, MV_a & V_ahp) {
						landmarkAHP::toBearingOnlyFrame(s, ahp, v, invDist, V_s, V_ahp);
				}

				/**
				 * AHP landmark from bearing-only retro-projection.
				 * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
				 * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
				 *
				 * It uses the formula (See Sola \e etal. PAMI 2010):
				 * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
				 *
				 * so that \a rho can be specified as being exactly inverse-distance.
				 *
				 * \param s the sensor frame
				 * \param v the retro-projected director vector in sensor frame
				 * \param rho the prior, proportional to inverse-distance
				 * \return the AHP landmark.
				 */
				template<class VS, class VLS>
				static vec7 fromBearingOnlyFrame(const VS & s, const VLS & v, const double _rho) {
						return landmarkAHP::fromBearingOnlyFrame(s, v, _rho);
				}

				/**
				 * AHP landmark from bearing-only retro-projection, with Jacobians.
				 * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
				 * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
				 *
				 * It uses the formula (See Sola \e etal. PAMI 2010):
				 * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
				 *
				 * so that \a rho can be specified as being exactly inverse-distance.
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
				static void fromBearingOnlyFrame(const VS & s, const VLS & v, const double _rho, VA & ahp, MA_s & AHP_s, MA_v & AHP_v, MA_rho & AHP_rho) {
						landmarkAHP::fromBearingOnlyFrame(s, v, _rho, ahp, AHP_s, AHP_v, AHP_rho);
				}


		}; // class LandmarkAnchoredHomogeneousPoint


	} // namespace rtslam
} // namespace jafar

#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_ */
