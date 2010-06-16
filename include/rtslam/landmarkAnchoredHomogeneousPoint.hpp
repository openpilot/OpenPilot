/**
 * \file landmarkAnchoredHomogeneousPoint.hpp
 * \author jsola@laas.fr
 * \date 14/02/2010
 *
 * Header file for anchored homogeneous point
 * \ingroup rtslam
 */

#ifndef LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_
#define LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_

#include "boost/shared_ptr.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"
#include "rtslam/ahpTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		class LandmarkAnchoredHomogeneousPoint;
		typedef boost::shared_ptr<LandmarkAnchoredHomogeneousPoint> ahp_ptr_t;



		/**
		 * Class for anchored homogeneous 3D points
		 * \ingroup rtslam
		 */
		class LandmarkAnchoredHomogeneousPoint: public LandmarkAbstract {
			public:


				/**
				 * Constructor from map
				 */
				LandmarkAnchoredHomogeneousPoint(const map_ptr_t & mapPtr);
				LandmarkAnchoredHomogeneousPoint(const simulation_t dummy, const map_ptr_t & mapPtr);

				virtual ~LandmarkAnchoredHomogeneousPoint() {
				}

				virtual std::string typeName() {
					return "Anchored homogeneous point";
				}


				virtual landmark_ptr_t convertToStandardParametrization()
				{
					// TODO
					//landmark_ptr_t lmk_euc = landmark_ptr_t(new LandmarkEuclideanPoint(??));
					//reparametrize_func(x(), lmk_euc->x(), jac);
					//lmk_euc->P() = compose(P(), jac);
					return landmark_ptr_t(new LandmarkAnchoredHomogeneousPoint(*this)); // FIXME this is not what we want
				}

				static size_t size(void) {
					return 7;
				}



				virtual size_t mySize() {return size();}

				virtual size_t stdSize() {return LandmarkAnchoredHomogeneousPoint::size();}

				virtual vec reparametrize_func(const vec & lmk){
					return lmkAHP::ahp2euc(lmk);
				}

				/**
				 * Reparametrize to Euclidean, with Jacobians.
				 * \param ahp the anchored homogeneous point to be reparametrized.
				 * \param euc the returned Euclidean point.
				 * \param EUC_ahp the Jacobian of the conversion.
				 */
				void reparametrize_func(const vec & ahp, vec & euc, mat & EUC_ahp) {
					lmkAHP::ahp2euc(ahp, euc, EUC_ahp);
				}

		}; // class LandmarkAnchoredHomogeneousPoint


	} // namespace rtslam
} // namespace jafar

#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINT_HPP_ */
