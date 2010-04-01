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

#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		/**
		 * Namespace for operations on Anchored Homogeneous Points
		 * \ingroup rtslam
		 */
		namespace landmarkAHP {
			using namespace ublas;
			/**
			 * Split AHP
			 * \param p0 the output anchor.
			 * \param m the output director vector
			 * \param rho the homogeneous parameter (inverse-distance)
			 */
			template<class AHP, class P0, class M>
			void split(const AHP & ahp, P0 & p0, M & m, double rho) {
				p0 = project(ahp, ublas::range(0, 3));
				m = project(ahp, ublas::range(3, 7));
				rho = ahp(7);
			}

			/**
			 * Compose AHP
			 * This is the opposite as split()
			 * \param p0 the output anchor.
			 * \param m the output director vector
			 * \param rho the homogeneous parameter (inverse-distance)
			 */
			template<class P0, class M>
			jblas::vec7 compose(const P0 & p0, const M & m, const double rho) {
				jblas::vec7 ahp;
				project(ahp, ublas::range(0, 3)) = p0;
				project(ahp, ublas::range(3, 7)) = m;
				ahp(7) = rho;
				return ahp;
			}

			template<class VF, class Vlf>
			jblas::vec fromFrame(const VF & F, const Vlf & ahpf) {

				// split non-trivial chunks of landmark state
				jblas::vec3 p0f(ublas::subrange(ahpf, 0, 3));
				jblas::vec3 mf(ublas::subrange(ahpf, 3, 6));

				// transformed landmark in global frame
				jblas::vec ahp(7);

				ublas::subrange(ahp, 0, 3) = quaternion::eucFromFrame(F, p0f);
				ublas::subrange(ahp, 3, 6) = quaternion::vecFromFrame(F, mf);
				ahp(6) = ahpf(6);

				return ahp;
			}

			template<class VF, class Vahpf, class Vahp, class MAHP_f, class MAHP_ahpf>
			void fromFrame(const VF & F, const Vahpf & ahpf, Vahp & ahp,
			    MAHP_f & AHP_f, MAHP_ahpf & AHP_ahpf) {
				// split non-trivial chunks of landmark state
				jblas::vec3 p0f(ublas::subrange(ahpf, 0, 3));
				jblas::vec3 mf(subrange(ahpf, 3, 6));

				// destination chunks
				jblas::vec3 p0;
				jblas::vec3 m;
				jblas::mat JAC_33(3, 3), JAC_37(3, 7);

				// Jacobians
				AHP_f.clear();
				AHP_ahpf.clear();

				// transform p0
				quaternion::eucFromFrame(F, p0f, p0, JAC_37, JAC_33);
				ublas::subrange(ahp, 0, 3) = p0;
				subrange(AHP_f, 0, 3, 0, 7) = JAC_37;
				subrange(AHP_ahpf, 0, 3, 0, 3) = JAC_33;
				// transform m
				quaternion::vecFromFrame(F, mf, m, JAC_37, JAC_33);
				subrange(ahp, 3, 6) = m;
				subrange(AHP_f, 3, 6, 0, 7) = JAC_37;
				subrange(AHP_ahpf, 3, 6, 3, 6) = JAC_33;
				// transform rho
				ahp(6) = ahpf(6);
				AHP_ahpf(6, 6) = 1;
			}

			template<class VF, class Vahp>
			jblas::vec toFrame(const VF & F, const Vahp & ahp) {

				// split non-trivial chunks of landmark state
				jblas::vec3 p0(ublas::subrange(ahp, 0, 3));
				jblas::vec3 m(subrange(ahp, 3, 6));

				// transformed landmark in frame F
				jblas::vec ahpf(7);

				ublas::subrange(ahpf, 0, 3) = quaternion::eucToFrame(F, p0);
				ublas::subrange(ahpf, 3, 6) = quaternion::vecToFrame(F, m);
				ahpf(6) = ahp(6);

				return ahpf;
			}

			template<class VF, class Vahp, class Vahpf, class MAHPF_f,
			    class MAHPF_ahp>
			void toFrame(const VF & F, const Vahp & ahp, Vahpf & ahpf,
			    MAHPF_f & AHPF_f, MAHPF_ahp & AHPF_ahp) {
				// split non-trivial chunks of landmark state
				jblas::vec3 p0(ublas::subrange(ahp, 0, 3));
				jblas::vec3 m(subrange(ahp, 3, 6));

				// destination chunks
				jblas::vec3 p0f;
				jblas::vec3 mf;
				jblas::mat JAC_33(3, 3), JAC_37(3, 7);

				// Jacobians
				AHPF_f.clear();
				AHPF_ahp.clear();

				// transform p0
				quaternion::eucToFrame(F, p0, p0f, JAC_37, JAC_33);
				ublas::subrange(ahpf, 0, 3) = p0f;
				subrange(AHPF_f, 0, 3, 0, 7) = JAC_37;
				subrange(AHPF_ahp, 0, 3, 0, 3) = JAC_33;
				// transform m
				quaternion::vecToFrame(F, m, mf, JAC_37, JAC_33);
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
			jblas::vec3 ahp2euc(const VA & ahp) {
				return ublas::subrange(ahp, 0, 3) + ublas::subrange(ahp, 3, 6) / ahp(6);
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
				jblas::vec3 m(subrange(ahp, 3, 6));
				double rho = ahp(6);
				jblas::identity_mat I(3);
				ublas::subrange(EUC_ahp, 0, 3, 0, 3) = I;
				ublas::subrange(EUC_ahp, 0, 3, 3, 6) = I / rho;
				ublas::column(EUC_ahp, 6) = -m / rho / rho;
			}

			/**
			 * Bring landmark to bearing-only sensor frame (without range information).
			 *
			 * For a landmark and sensor frame
			 * - ahp = [p0 m rho]
			 * - s = [t q],
			 *
			 * this function computes the chain:
			 * - R'(q) * ( m - (t - p0) * rho )
			 *
			 * which is a vector in sensor frame in the direction of the landmark. the range information is lost.
			 *
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \return the bearing-only landmark in sensor frame
			 */
			template<class VS, class VA>
			vec3 toBearingOnlyFrame(VS & s, VA & ahp) {
				vec3 p0, m;
				double rho;
				split(ahp, p0, m, rho);
				vec3 t = project(s, range(0, 3));
				vec4 q = project(s, range(3, 7));
				vec3 tmp = m - (t - p0) * rho;
				return quaternion::rotateInv(q, tmp);
			}

			/**
			 * Bring landmark to bearing-only sensor frame (without range information).
			 *
			 * For a landmark and sensor frame
			 * - ahp = [p0 m rho]
			 * - s = [t q],
			 *
			 * this function computes the chain:
			 * - R'(q) * ( m - (t - p0) * rho )
			 *
			 * which is a vector in sensor frame in the direction of the landmark. the range information is lost.
			 *
			 * and returns the Jacobians wrt s and ahp.
			 * \param s the sensor frame
			 * \param ahp the AHP landmark
			 * \param ls the bearing-only landmark in sensor frame
			 * \param LS_s the Jacobian of \a ls wrt \a s
			 * \param LS_ahp the Jacobian of \a ls wrt \a ahp
			 */
			template<class VS, class VA, class VLS, class MLS_s, class MLS_a>
			void toBearingOnlyFrame(VS & s, VA & ahp, VLS & ls, MLS_s & LS_s,
			    MLS_a & LS_ahp) {
				ls = toBearingOnlyFrame(s, ahp);
				vec3 p0, m;
				double rho;
				vec3 t = project(s, range(0, 3));
				vec4 q = project(s, range(3, 7));
				vec3 v = m - (t - p0) * rho; // before rotation
				mat34 LS_q;
				mat33 LS_v;
				quaternion::rotateInv(q, v, ls, LS_q, LS_v);
				ublas::subrange(LS_s, 0, 7, 0, 3) = rho * LS_v;
				ublas::subrange(LS_s, 0, 7, 4, 7) = LS_q;
				ublas::subrange(LS_ahp, 0, 3, 0, 3) = rho * LS_v; // dls / dp0
				ublas::subrange(LS_ahp, 0, 3, 3, 6) = LS_v; //       dls / dm
				ublas::column(LS_ahp, 7) = prod(LS_v, (t - p0)); //  dls / drho
			}

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
				LandmarkAnchoredHomogeneousPoint(MapAbstract & map);

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
				static jblas::vec fromFrame(const VF & F, const Vlf & ahpf) {
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
				template<class VF, class Vahpf, class Vahp, class MAHP_f,
				    class MAHP_ahpf>
				static void fromFrame(const VF & F, const Vahpf & ahpf, Vahp & ahp,
				    MAHP_f & AHP_f, MAHP_ahpf & AHP_ahpf) {
					landmarkAHP::fromFrame(F, ahpf, ahp, AHP_f, AHP_ahpf);
				}

				/**
				 * To-frame transform.
				 * \param F a frame to transform to
				 * \param ahp an AHP point in global frame
				 * \return the AHP point in F-frame
				 */
				template<class VF, class Vahp>
				static jblas::vec toFrame(const VF & F, const Vahp & ahp) {
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
				template<class VF, class Vahp, class Vahpf, class MAHPF_f,
				    class MAHPF_ahp>
				static void toFrame(const VF & F, const Vahp & ahp, Vahpf & ahpf,
				    MAHPF_f & AHPF_f, MAHPF_ahp & AHPF_ahp) {
					landmarkAHP::toFrame(F, ahp, ahpf, AHPF_f, AHPF_ahp);
				}

				/**
				 * Reparametrize to Euclidean.
				 * \param ahp the anchored homogeneous point to be reparametrized.
				 * \return the Euclidean point.
				 */
				template<class VA>
				static jblas::vec3 ahp2euc(const VA & ahp) {
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

		}; // class LandmarkAnchoredHomogeneousPoint


	} // namespace rtslam
} // namespace jafar

#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_ */
