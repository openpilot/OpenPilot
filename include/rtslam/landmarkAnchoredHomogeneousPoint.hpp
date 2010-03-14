/**
 * landmarkAnchoredHomogeneousPoint.hpp
 *
 *  Created on: 14/02/2010
 *      Author: jsola
 *
 * \file landmarkAnchoredHomogeneousPoint.hpp
 * \ingroup rtslam
 */

#ifndef LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_
#define LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_

#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"
#include "iostream"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		/**
		 * Class for anchored homogeneous 3D points
		 * \ingroup rtslam
		 */
		class Landmark3DAnchoredHomogeneousPoint: public LandmarkAbstract {
			public:

				/**
				 * Constructor from map and indirect array
				 */
				Landmark3DAnchoredHomogeneousPoint(MapAbstract & map, const jblas::ind_array & ial);

				static size_t size(void) {
					return 7;
				}


			private:

		}; // class Landmark3DAnchoredHomogeneousPoint

		/**
		 * Namespace for operations on Anchored Homogeneous Points
		 * \ingroup rtslam
		 */
		namespace landmarkAHP {

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
			void fromFrame(const VF & F, const Vahpf & ahpf, Vahp & ahp, MAHP_f & AHP_f, MAHP_ahpf & AHP_ahpf) {
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

			template<class VF, class Vahp, class Vahpf, class MAHPF_f, class MAHPF_ahp>
			void toFrame(const VF & F, const Vahp & ahp, Vahpf & ahpf, MAHPF_f & AHPF_f, MAHPF_ahp & AHPF_ahp) {
				//TODO: implement this
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

		}

	}
}

#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_ */
