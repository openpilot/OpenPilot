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
//#include "rtslam/ahpTools.hpp"

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
				vec fromFrame(const vec7 & F) {
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
				void fromFrame(const vec7 & F, vec & euc, mat & EUC_f, mat & EUC_eucf) {
					 quaternion::eucFromFrame(F, state.x(), euc, EUC_f, EUC_eucf);
				}


				/**
				 * To-frame transform.
				 * \param F a frame to transform to
				 * \param euc an Euclidean point in global frame
				 * \return the EUC point in F-frame
				 */
				vec toFrame(const vec7 & F) {
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
				void toFrame(const vec7 & F, vec & eucf, mat & EUCF_f, mat & EUCF_euc) {
						quaternion::eucToFrame(F, state.x(), eucf, EUCF_f, EUCF_euc);
				}
		}; // class LandmarkEuclideanPoint

				void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk){
					lnew = lmk;
				}


	} // namespace rtslam
} // namespace jafar


#endif /* LANDMARKEUCLIDEANPOINT_HPP_ */
