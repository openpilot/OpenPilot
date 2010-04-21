/*
 * landmarkEuclideanPoint.hpp
 *
 *  Created on: Apr 14, 2010
 *      Author: agonzale
 */

#ifndef LANDMARKEUCLIDEANPOINT_HPP_
#define LANDMARKEUCLIDEANPOINT_HPP_

#include "boost/shared_ptr.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/quatTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
	namespace rtslam {

		class LandmarkEuclideanPoint;
		typedef boost::shared_ptr<LandmarkEuclideanPoint> eucp_ptr_t;



		/**
		 * Class for Euclidean 3D points
		 * \ingroup rtslam
		 */
		class LandmarkEuclideanPoint: public LandmarkAbstract {
			public:


				/**
				 * Constructor from map
				 */
				LandmarkEuclideanPoint(const map_ptr_t & mapPtr);

				static size_t size(void) {
					return 3;
				}


				/**
				 * From-frame transform.
				 * \param F a frame to transform from
				 * \param eucf an Euclidean point in F-frame
				 * \return the Euclidean point in global frame
				 */
				template<class VF>
				vec fromFrame(const VF & F) {
					return quaternion::eucFromFrame(F,state.x());
				}


				/**
				 * From-frame transform, with Jacobians.
				 * \param F a frame to transform from
				 * \param eucf an Euclidean point in F-frame
				 * \param euc the Euclidean point in global frame
				 * \param EUC_f the Jacobian of \a euc wrt \a F
				 * \param EUC_eucf the Jacobians of \a euc wrt \a eucf
				 */
				template<class VF, class Vahp, class MAHP_f, class MAHP_ahpf>
				void fromFrame(const VF & F, Vahp & euc, MAHP_f & EUC_f, MAHP_ahpf & EUC_eucf) {
					 quaternion::eucFromFrame(F, state.x(), euc, EUC_f, EUC_eucf);
				}


				/**
				 * To-frame transform.
				 * \param F a frame to transform to
				 * \param euc an Euclidean point in global frame
				 * \return the EUC point in F-frame
				 */
				template<class VF>
				vec toFrame(const VF & F) {
					return quaternion::eucToFrame(F,state.x());
				}


				/**
				 * To-frame transform, with Jacobians.
				 * \param F a frame to transform to
				 * \param euc an Euclidean point in global frame
				 * \param eucf the Euclidean point in F-frame
				 * \param EUCF_f the Jacobian of \a eucf wrt \a F
				 * \param EUCF_euc the Jacobians of \a eucf wrt \a euc
				 */
				template<class VF, class Vahpf, class MAHPF_f, class MAHPF_ahp>
				void toFrame(const VF & F, Vahpf & eucf, MAHPF_f & EUCF_f, MAHPF_ahp & EUCF_euc) {
						quaternion::eucToFrame(F, state.x(), eucf, EUCF_f, EUCF_euc);
				}

				void reparametrize_func(const vec & euc, vec & euco, mat & EUC_euc){
					euco = euc;
				}



		}; // class LandmarkEuclideanPoint


	} // namespace rtslam
} // namespace jafar


#endif /* LANDMARKEUCLIDEANPOINT_HPP_ */
